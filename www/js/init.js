function init() { 
	gameDiv = document.getElementById("game");
	userList = document.getElementById("user_list");
	startGameForm = document.getElementById("start_game");
	ajaxStatus = document.getElementById("ajax_status");
	loginForm = document.getElementById("login_form");
	chatWindow = document.getElementById("chat_window");

	getInitResponse();
	printChatForm();
	setInterval('getResponse()', request_interval * 1000);
}
window.onload = init;
