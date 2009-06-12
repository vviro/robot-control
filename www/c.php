<?php
/**    
 * Server-side response and logic
 *    
 * PHP version 5
 *
 * @category PHP
 * @package  Game
 * @author   Vladimir Viro <vviro@mail.ru>
 * @license  BSD License
 * @link     http://seoka.de/game
 */

include "inc/sess.inc";
include "inc/db.inc";

include "inc/inputvars.inc";

if (isset($_SESSION['c'])) {
    $c = $_SESSION['c'];
} else {
    $c = array();
}

$c_old = $c;

$username = $c['username'];

if (isset($partner)) { 
    $c['partner'] = $partner;
    play_with($username, $partner);
}
else $partner = ($c['partner'] == null?null:$c['partner']);

if (isset($partner)) {
    $ready = partner_ready($username, $partner);
    $c['ready'] = $ready;
}

if (isset($ltime)) {
    $time_diff = round($ltime/1000) - $_SERVER['REQUEST_TIME'];
    $c['time_diff'] = $time_diff;
    set_timediff($time_diff, $username);
} else $time_diff = $c['time_diff'];

if (isset($message)) {
    $message = mysql_real_escape_string($message);
    $recepient = mysql_real_escape_string($recepient);
    send_message($recepient, $message, $username);
}
if (isset($solution)) {
    send_solution($solution, $username);
}

$c['messages']    = get_messages($self);
$usersonline      = userlist($username);
$c['usersonline'] = $usersonline;
$cond             = give_conditions($username);
$c['cond']        = $cond;

/// Stop changing the session variable, compare it to the old state, generate a response and save in DB

$c['md5_hash']     = '';
$c_old['md5_hash'] = '';
$old_session_hash  = md5(serialize($c_old));
$new_session_hash  = md5(serialize($c));

//if (isset($_GET['last_seen_session_hash']) and $_GET['last_seen_session_hash'] != $old_session_hash) {
//    $sql = "select ses_value from session_buffer where ses_hash = '".$_GET['last_seen_session_hash']."'";
//    $q = mysql_query($sql);
//    if (mysql_num_rows($q)>0) $c_old = mysql_result($q,0);
//}

$c_old['md5_hash'] = $old_session_hash;
$c['md5_hash']     = $new_session_hash;

$send_all = $_GET['init'];

if (isset($_GET['last_seen_session_hash']) and $_GET['last_seen_session_hash'] != $old_session_hash) {
    $redraw = 1;
} else $redraw = 0;

$response = form_diff_response($c, $c_old, $send_all, $redraw);

$_SESSION['c'] = $c;

function form_diff_response($c_new, $c_old, $send_all, $redraw) 
{
    include 'utils/array_diff.php';
    $c_diff_new = arrDiff($c_new, $c_old);
    $c_diff_old = arrDiff($c_old, $c_new);

    $response = array();
    if ($send_all==1) {
        $response['in'] = $c_new;
    } else {
        if (count($c_diff_new)>0) $response['in'] = $c_diff_new;
        if (count($c_diff_old)>0) $response['out'] = $c_diff_old;
    }
    if ($redraw == 1) {
        $response['in'] = $c_new;
        $response['in']['redraw'] = 1;
    }
    return $response;
}

function set_timediff($time_diff, $self) 
{
    include "inc/db.inc";
    $sql = "update sessions set time_diff = $time_diff where uid = (select uid from users where username = '$self')";
    mysql_query($sql, $mysql_access);
}

function userlist($self) 
{
    include "inc/db.inc";

    //* collect list of users who were seen in last 15 seconds and are logged in
    $sql = "select username from sessions inner join users on users.uid = sessions.uid ".
           "where unix_timestamp() - ses_time < 15 and ses_id != '".session_id()."' order by username asc";
    $q   = mysql_query($sql, $mysql_access);

    //* select users, that want to play with me
    $sql = "select u2.username from users as u1 inner join sessions as s inner join users as u2 ".
           "on s.uid = u2.uid and u1.uid = s.partner_uid and u1.username = '$self' order by u2.username";

    $q2 = mysql_query($sql, $mysql_access);
    $wannaplaywithme = array();
    if (mysql_num_rows($q2)>0) while ($r = mysql_fetch_row($q2)){
        $wannaplaywithme[] = $r[0];
    }

    $sql = "select u2.username from users as u1 inner join users_opponents as uop1 ".
        "inner join users_opponents as uop2 inner join users as u2 ".
        "on uop1.uid = u1.uid and u2.uid = uop2.uid and uop1.oppid = uop2.oppid ".
        "and u1.username = '$self' and uop1.active = 1 and uop1.uid != uop2.uid order by u2.username";

    $q3 = mysql_query($sql, $mysql_access);
    $playswithme = array();
    if (mysql_num_rows($q3)>0) while ($r = mysql_fetch_row($q3)){
        $playswithme[] = $r[0];
    }

    $usersonline = array();
    while($r = mysql_fetch_row($q)) {
        if (in_array($r[0], $wannaplaywithme)) $a = 1; 
        else $a = 0;
        if (in_array($r[0], $playswithme)) $a = 2;
        $usersonline[htmlspecialchars($r[0])] = array('activity'=>$a);
    }

    return $usersonline;
}

function partner_ready($self, $partner) 
{
    include "inc/db.inc";

    /** check if there is a partner with this name, if not, say not ready **/
    if (mysql_num_rows(mysql_query("select uid from users where username = '$partner'", $mysql_access))>0) {

    /** change user status to "Wannaplay" with partner $partner **/
        $sql = "update sessions set partner_uid = (select if('$partner' != '',(select uid from users where username = '$partner'),0)), ".
               "wannaplay = 1 where uid = (select uid from users where username = '$self')";
        mysql_query($sql, $mysql_access);

        /** see if the partner wants to play with me **/
        $sql = "select count(*) from users as u1 inner join sessions as s inner join users as u2 ".
               "on u1.uid = s.uid and u2.uid = s.partner_uid ".
           "where u1.username = '$partner' and u2.username = '$self'";
        $ready = mysql_result(mysql_query($sql, $mysql_access),0);
    }

    if ($ready >= 1) $ready = 1;
    else $ready = 0;
    
    return $ready;
}

function play_with($self, $partner) 
{
    include "inc/db.inc";

    /** check if my partner is ready **/
    if (partner_ready($self, $partner)) {
        /** create a new opponents entity or join existing one **/

        /** if the partner is already waiting, assume he has created the opponents-record, so we just join it **/
        $sql = "select oppid from users_opponents where uid = (select uid from users where username = '$partner') and active = 1";
        $oppid = mysql_result(mysql_query($sql, $mysql_access),0);
        
        // if I was active in other opponents-records, set active there to 0 
        $sql = "update users_opponents set active = 0 ".
            "where active = 1 and uid = (select uid from users where username = '$self') and oppid != $oppid";
        mysql_query($sql, $mysql_access);

        // join the opponents-record 
        $sql = "insert into users_opponents (oppid, uid) values ('$oppid', (select uid from users where username = '$self')) ";
        mysql_query($sql, $mysql_access);

        /** now we can create a new game **/
        // check if there already exist an actual game with this oppid, if not - create new one and take its gameid
        $sql = "select gameid from games where oppid = '$oppid' and unix_timestamp(start_time) + timelimit > unix_timestamp(now())";
        $q = mysql_query($sql, $mysql_access);
        if (mysql_num_rows($q)>0) {
            $gameid = mysql_result($q,0);
        }
        else {
            $gameid = create_game($oppid);
        }
    } else {
        /** look if there exists oponents-record where I am the only active opponent **/
        $sql = "select oppid, count(*) as c from users_opponents where oppid in ".
            "(select oppid from users_opponents where active = 1 and uid = (select uid from users where username = '$self')) ".
            "group by oppid having c = 1";
        $q = mysql_query($sql, $mysql_access);
        if (mysql_num_rows($q)>0) {
            /** if yes, choose its oppid **/
            $oppid = mysql_result($q,0);
        } else {
            /** if no, create a new one and take its oppid **/
            $sql = "insert into opponents (time_formed) values (now())";
            mysql_query($sql, $mysql_access);
            $oppid = mysql_insert_id();
        }

        // if I was active in other opponents-records, set active there to 0 
        $sql = "update users_opponents set active = 0 ".
            "where active = 1 and uid = (select uid from users where username = '$self') and oppid != $oppid";
        mysql_query($sql, $mysql_access);

        /** write to users_opponents: if already done, nothing changes, if new opponents, change oppid **/
        $sql = "insert into users_opponents (oppid, uid) values ('$oppid', (select uid from users where username = '$self'))";
        mysql_query($sql, $mysql_access);
    }
}

function create_game($oppid) 
{
    include("inc/db.inc");

    $sql = "select phrase from phrase_pool order by rand() limit 2";
    $q = mysql_query($sql, $mysql_access);
    $start_phrase = mysql_result($q,0);
    $end_phrase = mysql_result($q,1);

    $sql = "insert into games (oppid, start_time, start_phrase, end_phrase, timelimit) ".
        "values ('$oppid', now() + interval 10 second, '$start_phrase', '$end_phrase', 60)";
    mysql_query($sql, $mysql_access);
    return mysql_insert_id();
}

function after_the_game($oppid) {};

function give_conditions($self) 
{
    include "inc/db.inc";
    $sql = "select gameid from games natural join opponents natural join users_opponents natural join users where username = '$self'";
    $q = mysql_query($sql, $mysql_access);
    if (mysql_num_rows($q)>0) {
        $gameid = mysql_result($q,0);

        $sql = "select start_phrase, end_phrase, unix_timestamp(start_time), timelimit from games where gameid = $gameid";
        $r = mysql_fetch_row(mysql_query($sql, $mysql_access));

        $cond = array();

        $cond['start_phrase'] = $r[0];
        $cond['end_phrase'] = $r[1];
        $cond['game_length'] = $r[3];
        $cond['game_start_delay'] = 0; //* game start time + 0 seconds *//
        $cond['game_start_time'] = $r[2];
    }
    return $cond;
}

function send_message($recepient, $message, $self) 
{
    include("inc/db.inc");
    $sql = "insert into messages (sender_uid, recepient_uid, body) values ((select uid from users where username = '$self'), ".
        "(select uid from users where username = '$recepient'), '$message')";
    mysql_query($sql, $mysql_access);
}

function get_messages($self) 
{
    include "inc/db.inc";
    $sql =  "select username, body, timestamp, messid from (".
        "select u2.username, body, messages.timestamp, messages.messid from users as u1 inner join messages inner join users as u2 ".
        "on u2.uid = messages.sender_uid and messages.recepient_uid = u1.uid ".
        "where now() - interval 30 minute < messages.timestamp ".
        "order by messages.timestamp desc limit 10 ".
        ") as t order by timestamp asc";
    $q        = mysql_query($sql, $mysql_access);
    $messages = array();
    if (mysql_num_rows($q)>0)
    while ($r = mysql_fetch_row($q)) {
        $m['sender'] = $r[0];
        $m['body'] = htmlspecialchars($r[1]);
        $m['timestamp'] = $r[2];
        $messages[$r[3]] = $m;
    }
    return $messages;
}

function send_solution($solution, $self) 
{
    require "inc/db.inc";
    $sql = "select gameid from games natural join opponents natural join users_opponents natural join users where username = '$self'";
    $q   = mysql_query($sql, $mysql_access);
    if (mysql_num_rows($q)>0) {
        $gameid = mysql_result($q, 0);
        $sql    = "insert into games_log (gameid, uid, solution) ".
            "values ($gameid, (select uid from users where username = '$self'), '$solution') ".
            "on duplicate key update solution = '$solution'";
        mysql_query($sql, $mysql_access);
    }
}


header('Content-Type: text/xml');
header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");  // disable IE caching
header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT"); 
header("Cache-Control: no-cache, must-revalidate"); 
header("Pragma: no-cache");
require 'utils/assoc_array2xml.php';
$converter    = new assoc_array2xml;
$response_xml = $converter->array2xml($response);
print($response_xml);

