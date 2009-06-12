<?
if (!isset($_SESSION['c']['username']))
print("
    <form id='login_form' action='javascript:sendLogin()'>
	    <fieldset>
	    <label for 'username'>Please enter your login...</label><br>
	    <input type='text' name='username' size='10' onKeypress='if (window.event && window.event.keyCode == 13) sendLogin()' />
	    <input type='submit' value='login'/>
	    </fieldset>
    </form>
");
else
print("
<div id='login_form'>
You are loggen in as <b class='username'>".$_SESSION['c']['username']."</b>
<br><a href='logout.php'>logout</a>
</div>
");
