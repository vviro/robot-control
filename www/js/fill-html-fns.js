
function printLoggedIn(username) {
	removeChildren(loginForm);
	loginForm.appendChild(document.createTextNode("You are logged in as "));
	var uname = document.createElement('b');
	uname.className = 'username';
	uname.appendChild(document.createTextNode(username));
	loginForm.appendChild(uname);
	loginForm.appendChild(document.createElement("br"));
	var logoutLink = document.createElement('a');
	logoutLink.setAttribute('href', 'logout.php');
	logoutLink.appendChild(document.createTextNode("logout"));
	loginForm.appendChild(logoutLink);
}

function printUserList() {
	removeChildren(userList);
	for (user in users_online) {
		sid = document.createElement("b");
		sid.className = "username";
		var textContent = users_online[user].username; 
		sid.appendChild(document.createTextNode(textContent));
		userList.appendChild(sid);
		
		var play = document.createElement('span');
		if (users_online[user].activity == 1) {
			play.appendChild(document.createTextNode(" (wants to play with you)"));
		} else if (users_online[user].activity == 2) {
			play.appendChild(document.createTextNode(" (plays with you)"));
		}
		play.style.fontSize = '85%';
		if (users_online[user].activity != 0) userList.appendChild(play);
		
		userList.appendChild(document.createElement('br'));
	}
}

function printWaitingForPartner(partner) {
	removeChildren(gameDiv);
	gameDiv.appendChild(document.createTextNode('Waiting for '));
	var uname = document.createElement('b');
	uname.className = 'username';
	uname.appendChild(document.createTextNode(partner));
	gameDiv.appendChild(uname);
}

function printChatForm() {
	var chatTitle = document.createElement('div');
	chatTitle.id = 'chat_title';
	chatTitle.style.fontStyle = 'italic';
	chatTitle.appendChild(document.createTextNode('small chat'));
	chatWindow.appendChild(chatTitle);

	var chatHistory = document.getElementById('chat_history');
	chatHistory = document.createElement('div');
	chatHistory.id = 'chat_history';
	chatWindow.appendChild(chatHistory);

	var chatForm = document.getElementById('chat_form');
	if (chatForm == null) {
		var chatForm = document.createElement('form');
		chatForm.id = 'chat_form';
		chatForm.action = 'javascript:sendMessage()';
		var chatInputField = document.createElement('input');
		chatInputField.name = 'chatInput';
		chatInputField.id = 'chat_form_input';
		chatInputField.type = 'text';
		chatInputField.setAttribute('size','30');

		chatForm.appendChild(chatInputField);
		chatWindow.appendChild(chatForm);
	}
}

function printChatMessage(message) {
	var chatHistory = document.getElementById('chat_history');
	var sender = document.createElement('b');
	sender.className = 'username';
	sender.appendChild(document.createTextNode(message.sender));
	chatHistory.appendChild(sender);
	
	chatHistory.appendChild(document.createTextNode(": " + message.body));
	chatHistory.appendChild(document.createElement('br'));
	removeOldMessages();
}

function printGameConditions() {
	var gameConditions = document.createElement('div');
	gameConditions.id = 'game_conditions';
	gameConditions.appendChild(document.createTextNode('connect "' + start_phrase + '" with "' + end_phrase + '"'));

	var gameInput = document.createElement('form');
	gameInput.id = 'game_input';
	gameInput.action = 'javascript:sendSolution()';
	var textfield = document.createElement("input");
	textfield.type = 'text';
	textfield.name = 'solution';
	textfield.id = 'game_solution_input';
	gameInput.appendChild(document.createTextNode(start_phrase+" "));
	gameInput.appendChild(textfield);
	gameInput.appendChild(document.createTextNode(end_phrase));
	
	removeChildren(document.getElementById("count_down"));
	gameDiv.appendChild(gameConditions);
	gameDiv.appendChild(gameInput);
}

function printGameOver() {
	document.getElementById('count_down').appendChild(document.createTextNode("Your time is up! Game Over!"));
}
