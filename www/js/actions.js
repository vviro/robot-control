
function waitForPartner() {
	var playwith = startGameForm.elements['playwith'].value;
	if (playwith == "somebody") { playwith = "";}
	var params = new Array();
	params['playwith'] = playwith;
	params['ltime'] = (new Date()).getTime();
	getResponse(params);
}

function sendMessage() {
	var mbody = document.getElementById('chat_form_input').value;
	var recepient = '';
	var params = new Array();
	params['message_recepient'] = recepient;
	params['message_body'] = mbody;
	params['request_method'] = 'POST';
	if (mbody.replace(/^\s+|\s+$/g,"").length > 0) getResponse(params);

	document.getElementById('chat_form_input').value = '';
}

function sendSolution() {
	var sol = document.getElementById('game_solution_input').value;
	var params = new Array();
	params['sol']=sol;
	params['request_method']='POST';
	getResponse(params);
}
