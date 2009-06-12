function clickclear(thisfield, defaulttext) {
if (thisfield.value == defaulttext) {
thisfield.value = "";
}
}

function clickrecall(thisfield, defaulttext) {
if (thisfield.value == "") {
thisfield.value = defaulttext;
}
}

removeChildren = function(target) {
	if (target.hasChildNodes()){
		while (target.firstChild) {
			target.removeChild(target.firstChild);
		}
	}
}

function removeOldMessages() {
	var chatHistory = document.getElementById('chat_history');
	var text = chatHistory.innerHTML;
	var splitArr = text.toLowerCase().split(/<br/g);
	var lineCount = splitArr.length - 1;
	if (lineCount>chatLength) {text = text.substring(text.toLowerCase().indexOf('<br>')+4,text.length);}
	chatHistory.innerHTML = text;
}
