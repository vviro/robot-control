<?php
/**	
 * Cleaning input variables
 *	
 * PHP version 5
 *
 * @category PHP
 * @package  Game
 * @author   Vladimir Viro <vviro@mail.ru>
 * @license  BSD License
 * @link     http://seoka.de/game
 */

if (get_magic_quotes_gpc()) {
    $partner   = stripslashes($_REQUEST['playwith']);
    $ltime     = stripslashes($_REQUEST['ltime']);
    $message   = stripslashes($_REQUEST['message_body']);
    $recepient = stripslashes($_REQUEST['message_recepient']);
    $solution  = stripslashes($_REQUEST['sol']);
} else {
    $partner   = $_REQUEST['playwith'];
    $ltime     = $_REQUEST['ltime'];
    $message   = $_REQUEST['message_body'];
    $recepient = $_REQUEST['message_recepient'];
    $solution  = $_REQUEST['sol'];
}

if (!isset($_REQUEST['playwith'])) $partner = null;
if (!isset($_REQUEST['ltime'])) $ltime = null;
if (!isset($_REQUEST['message_body'])) $message = null;
if (!isset($_REQUEST['message_recepient'])) $recepient = null;
if (!isset($_REQUEST['sol'])) $solution = null;

?>
