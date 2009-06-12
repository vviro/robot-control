
function process_response(o) {
	
	response = o.responseXML.documentElement;
	
	if (response.getElementsByTagName('out').length > 0) {
	
	var neg = response.getElementsByTagName('out')[0];

	// who is online and wants to play with me
	if (neg.getElementsByTagName('usersonline').length > 0) {
		var users_out_test = neg.getElementsByTagName("usersonline");
		if (users_out_test.length > 0) {
			users_out = users_out_test[0].childNodes;
		} else { users_out = new Array();}
		for (var j = 0; j < users_out.length; j++) {
			var x = users_out[j];
			var user = new Object();
			user.username = x.nodeName;
			user.activity = x.getElementsByTagName("activity")[0].firstChild.data;
			for(i=0;i<users_online.length;i++){
				if(user.username==users_online[i].username) {
					users_online.splice(i, 1);
				}
			}
		}
	}
	
	}	
	
	
	if (response.getElementsByTagName('in').length > 0) {

	var pos = response.getElementsByTagName('in')[0];
    
	if (response.getElementsByTagName('redraw').length>0) {
		removeChildren(userList);
		users_online = Array();
		var chat_history = document.getElementById("chat_history");
		removeChildren(chat_history);

	}

	if (pos.getElementsByTagName('usersonline').length > 0) {
		var users_in_test = pos.getElementsByTagName("usersonline");
		if (users_in_test.length > 0) {
			users_in = users_in_test[0].childNodes;
		} else { users_in = new Array();}
		for (var j = 0; j < users_in.length; j++) {
			var x = users_in[j];
			var user = new Object();
			user.username = x.nodeName;
			user.activity = x.getElementsByTagName("activity")[0].firstChild.data;
			users_online.push(user);
		}
	}
	printUserList();	


	if (response.getElementsByTagName('in').length > 0) {
		if (pos.getElementsByTagName("md5_hash").length > 0) {
			last_seen_session_hash = pos.getElementsByTagName("md5_hash")[0].firstChild.data;
		}
		if (pos.getElementsByTagName("time_diff").length > 0) {
			time_diff = parseInt(pos.getElementsByTagName("time_diff")[0].firstChild.data);
		}
		if (pos.getElementsByTagName("partner").length > 0) {
			partner = pos.getElementsByTagName("partner")[0].firstChild.data;
		}
		if (pos.getElementsByTagName("ready").length > 0) {
			ready = pos.getElementsByTagName("ready")[0].firstChild.data;
		}
		
		if (ready == "1") {
			if (pos.getElementsByTagName("start_phrase").length > 0) {
				start_phrase = pos.getElementsByTagName("start_phrase")[0].firstChild.data;
			}
			if (pos.getElementsByTagName("end_phrase").length > 0) {			
				end_phrase = pos.getElementsByTagName("end_phrase")[0].firstChild.data;
			}
			if (pos.getElementsByTagName("game_length").length > 0) {
				game_length = parseInt(pos.getElementsByTagName("game_length")[0].firstChild.data);
			}
			if (pos.getElementsByTagName("game_start_time").length > 0) {
				game_start_time = parseInt(pos.getElementsByTagName("game_start_time")[0].firstChild.data);
			}
			if (pos.getElementsByTagName("game_start_delay").length > 0) {	
				game_start_delay = parseInt(pos.getElementsByTagName("game_start_delay")[0].firstChild.data);
			}

			document.getElementById('start_game_div').style.display = 'none';

			var countdown = document.createElement('div');
			countdown.id = 'count_down';
			removeChildren(gameDiv);
			gameDiv.appendChild(countdown);
			timeBeforeStart = (game_start_delay + game_start_time - Math.round((new Date()).getTime()/1000)+ time_diff);
			timeBeforeEnd = (game_length + game_start_time - Math.round((new Date()).getTime()/1000)+ time_diff);
			timerElement = countdown;
			if (timeBeforeStart >= 0) {
				timerLimit = timeBeforeStart;
				timeoutFunction = giveConditions;
				timerText = "the game starts in ";
				startTimer();
			} else
			if (timeBeforeStart < 0 && timeBeforeEnd > 0) {
				timeoutFunction = closeRound;
				timerLimit = timeBeforeEnd;
				timerText = "you have ";
				giveConditions();
			} else
			{
				timeoutFunction = closeRound;
				timerLimit = 0;
				timerText = "you have ";
				giveConditions();
			}
		} else
		if (ready != "1" && partner != undefined) {
			printWaitingForPartner(partner);
		}

		if (pos.getElementsByTagName("messages").length > 0) {

			var messages_list = pos.getElementsByTagName("messages")[0].childNodes;
			if (messages_list.length > 0) {
				var messages = pos.getElementsByTagName("messages")[0].childNodes;
			}
			for (var j = 0; j < messages.length; j++) {
				var x = messages[j];
				var message = new Object();
				if (x.getElementsByTagName("timestamp").length > 0) 
					{message.timestamp = x.getElementsByTagName("timestamp")[0].firstChild.data;}
				if (x.getElementsByTagName("body").length > 0) 
					{message.body = x.getElementsByTagName("body")[0].firstChild.data;}
				if (x.getElementsByTagName("sender").length > 0) 
					{message.sender = x.getElementsByTagName("sender")[0].firstChild.data;}
				printChatMessage(message);
			}
		}
	}
	
	}
}
