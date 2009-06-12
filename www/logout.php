<?php
/**	
 * Logout trigger
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

$username = $_SESSION['c']['username'];

session_destroy();

$sql = "delete from sessions where uid = (select uid from users where username = '$username')";
mysql_query($sql, $mysql_access);

$location = '/robot-control';
header("Location: $location");
print("<html><head><meta http-equiv='refresh' content='0; url=$location'></head></html>");
exit;
?>
