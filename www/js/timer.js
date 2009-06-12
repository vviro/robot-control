
updateTimer = function() {
	if(timerID) clearTimeout(timerID);
	if(!tStart) tStart   = new Date();

	var tDate = new Date();
	var tDiff = tDate.getTime() - tStart.getTime();
	tDate.setTime(tDiff);

	this.timerElement.innerHTML = timerText +
	(timerLimit - tDate.getMinutes()*60 - tDate.getSeconds()) + " seconds";

	timerID = setTimeout("updateTimer()", 1000);
	if (tDate.getMinutes()*60 + tDate.getSeconds() >= timerLimit) {
		stopTimer();
		timeoutFunction();
	}
}

startTimer = function(){
	tStart = new Date();
	this.timerElement.innerHTML = timerText + timerLimit + " seconds";
	setTimeout("updateTimer()", 1000); 
}

stopTimer = function() {
	if(timerID) {
		clearTimeout(timerID);
		timerID  = 0;
	}
	tStart = null;
}

resetTimer = function() {
	tStart = null;
	this.timerElement.innerHTML = timerText + timerLimit + " seconds";
}

