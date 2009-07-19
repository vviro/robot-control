// connection variables
var request_interval = 4;
var time_diff;

// layout constants - DIVs

var gameDiv;
var userList;
var startGameForm;
var ajaxStatus; 
var loginForm; 
var chatWindow;

// system variables

var users_online = new Array();
var messages = new Array();
var chatLength = 5;
var last_seen_session_hash;

// self variables

var username;

// partner variables
var partner;
var ready = 0;

// game variables
var start_phrase = 'start';
var end_phrase = 'end';
var game_length = 0;
var game_start_time;
var game_start_delay;

// timer variables
var timerID = 0;
var tStart  = null;
var timerLimit = 0;
var timerElement;
var timeoutFunction;
var timerText = '';
