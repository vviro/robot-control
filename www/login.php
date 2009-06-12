<?php
/**	
 * Login handler
 *	
 * PHP version 5
 *
 * @category PHP
 * @package  Game
 * @author   Vladimir Viro <vviro@mail.ru>
 * @license  BSD License
 * @link     http://seoka.de/game
 */

require_once "inc/sess.inc";
require_once "inc/db.inc";

$username = mysql_real_escape_string($_GET['username']);
$ses_id   = session_id();

$sql = "select username, status from users where username = '".$username."' limit 1";
$q   = mysql_query($sql, $mysql_access);

if (mysql_num_rows($q)>0) {
    $r        = mysql_fetch_row($q);
    $username = $r[0];
    $status   = $r[1];
} else {
    $sql = "insert into users (username, status) values ('$username','guest')";
    mysql_query($sql);
    $username = $username;
    $status   = 'guest';
}

$sql = "update sessions set uid = (select uid from users ".
       "where username = '$username' limit 1) where ses_id = '$ses_id'";
mysql_query($sql, $mysql_access);

$_SESSION['c']['authentificated'] = true;
$_SESSION['c']['status']          = $status;
$_SESSION['c']['username']        = $username;

header("Content-Type:text/xml");

print('<?xml version="1.0"?>'."\n");
print("<response>");
print("<success>");
if (isset($username)) print("1"); else print("0");
print("</success>");
if (true) {
    print("<username>".$username."</username>");
    print("<status>".$status."</status>");
}
print("</response>");
?>
