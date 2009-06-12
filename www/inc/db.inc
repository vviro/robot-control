<?php
/**	
 * Database Connection snippet
 *	
 * PHP version 5
 *
 * @category PHP
 * @package  Game
 * @author   Vladimir Viro <vviro@mail.ru>
 * @license  BSD License
 * @link     http://seoka.de/game
 */

$mysql_access = mysql_connect("localhost", "root", "rootpass");
mysql_select_db("game");
mysql_query("SET NAMES 'utf8'", $mysql_access);
?>
