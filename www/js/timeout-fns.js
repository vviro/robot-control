
function giveConditions() {
	printGameConditions();
	
	timeoutFunction = closeRound;
	timeBeforeEnd = (game_length + game_start_time - Math.round((new Date()).getTime()/1000)+ time_diff);
	timerLimit = timeBeforeEnd;
	timerText = "you have ";
	startTimer();
}

function closeRound() {
	if (document.getElementById("count_down")) removeChildren(document.getElementById("count_down"));
	sendSolution();
	printGameOver();
	document.getElementById('start_game_div').style.display = 'block';
}
