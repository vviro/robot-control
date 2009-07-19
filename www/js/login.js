var loginCallback = {
    success:function(o) {
	var loginForm = document.getElementById("login_form");
	var response = o.responseXML.documentElement;
	var x = response.getElementsByTagName("success");
	if (x[0].firstChild.data == "1") {
	    var status = response.getElementsByTagName("status")[0].firstChild.data;
	    username = response.getElementsByTagName("username")[0].firstChild.data;

	    printLoggedIn(username);
	}
    }
    ,
    failure:function(o) {
	ajaxStatus.innerHTML = o.status + " " + o.statusText; 
    }
}

function sendLogin() {
	var loginForm = document.getElementById("login_form");
	var username = loginForm.elements['username'].value;
	var entryPoint = 'login.php';
	var queryString = encodeURI('?username=' + username);
	var sUrl = entryPoint + queryString;
	var request = YAHOO.util.Connect.asyncRequest('GET', sUrl, loginCallback);
}
