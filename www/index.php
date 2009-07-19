<?php
/**	
 * Index.php
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
?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">
<html>

<head>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
	<link rel="stylesheet" type="text/css" href="css/site_<?php require 'utils/combine_css.php'; ?>.css">
	<script type="text/javascript" src="js/site_<?php require 'utils/combine_jsmin.php'; ?>.js"></script>
	<title>Robotic Control Panel</title>
</head>

<body>
	<div id='wrap'>
	<div id='fader'>

	<div id='doc2' class="yui-t2">
   
		<div id='hd'>
			<div id='logo'>
				<a href='/robot-control'>
					<h2>Robotic Control Panel</h2>
				</a>
			</div>
			<a id='tellafriend' href='javascript:tellafriend();'>Invite a Friend</a>
			<?php require_once "snippets/login_form.php"; ?>
			<div style="clear:both;"></div>
		</div>
   
		<div id='bd'>
			<div id="yui-main">  
				<div class="yui-b">
					<div class="yui-gc">  
						<div class="yui-u first" id="game_old">  
							<?php //require_once "snippets/params.php"; ?>
                                                        <img src="http://www-users.cs.umn.edu/%7Emartin/images/torque.bmp" width="503"/>
						</div>  
						<div class="yui-u" id="right_part"> 
                                                        <p><i>&nbsp; also online:</i>
                                                        <div id="user_list"></div>

							<div id='chat_window'></div>
						</div>
					</div> <!-- yui-g -->
					<div class="yui-g" id="game">  
						<!-- YOUR DATA GOES HERE -->  
					</div>  
				</div> <!-- yui-b -->
			</div> <!-- yui-main -->
   
			<div class="yui-b" id="side_tab"> <!-- side tab -->
                                <?php require_once "snippets/params.php"; ?>

				<div id='ajax_status'></div>
			</div>

		</div> <!-- bd -->
   
		<div id='ft'>
<!=--		&copy; Vladimir Viro -->
		</div>

	</div> <!-- doc2 -->
	
	</div> <!-- fader -->
	</div> <!-- wrap -->
</body>

</html>
