
var processResponseCallback = {
    success:function(o) {
	process_response(o);
	ajaxStatus.innerHTML = '';
    }
    ,
    failure:function(o) {
        ajaxStatus.innerHTML = o.status + " " + o.statusText;
    }
}

function getResponse(params) {
	if (params == null) params = new Array();
	
	var base = 'c.php';
	var method = 'GET';
	var p = '';
	var postMessage = null;

	if (params['request_method'] != 'POST') {
		for (key in params) {
			if (key != 'request_method') {p = p + key + '=' + params[key] + '&';}
			else method = params[key];
		}
	} else {
		for (key in params) {
			if (key != 'request_method') {p = p + key + '=' + params[key] + '&';}
			else method = params[key];
		}
		postMessage = p;
		p = '';
	}
	var sUrl = base + "?" + p;
	if (params.length == 0) {
		sUrl = sUrl + "&last_seen_session_hash=" + last_seen_session_hash + '&';
	}

	sUrl = sUrl.substring(0,sUrl.length-1); /// deletes "&" at the end...
	var request = YAHOO.util.Connect.asyncRequest(method, sUrl, processResponseCallback, postMessage);
}

function getInitResponse() {
	getResponse({'init':'1'});
}
