var plot_divs = [];
var should_update = true;
var period = 300;

function set_last_update() {
    should_update = false;
    var this_timer_id = Math.random(10000);
    timer_id = this_timer_id;
    $.timer(period/2, function (timer) {
        if (timer_id == this_timer_id) {
            should_update = true;
        }
        timer.stop();
    });
}

//$(document).ready(function() {
    var url = "fcgi/test.fcgi";
    var md = 400;
    
    var svgdiv = $('#svgdiv');
    var gg;
    $.timer(period, function (timer) {

        if (should_update) {
            set_last_update();
	    
            $.post(url, { 'action': 'get_data_stream' }, function(xml) { 

    		$(xml).find('param').each(function() {
		    var name = $(this).find('name').text();
		    var x2 = $(this).find('x').text();
		    var y2 = $(this).find('y').text();

		    var div = $('#svg_' + plot_divs[name]);
		    var mult_c = $('#svg_' + plot_divs[name] + '_mult').val();
		    mult_c = (mult_c !== undefined ? mult_c : 1);

		    if (plot_divs[name] === undefined || div.children().size() === 0 ) return;

		    var svg = div.data('svg');
		    var g = div.data('g');
		    

		    var x1 = (div.data('last_x') === undefined ? x2 : div.data('last_x'));
		    var y1 = (div.data('last_y') === undefined ? y2 : div.data('last_y'));


		    x2 = x2 * 4;
		    x2 = x2 % md;

		    //var g = svg.group({stroke: 'black', 'stroke-width': 2});	
		    if (x2 < x1) {
		    	x1 = x2;
			y1 = y2;	
			div.data('last_x',0);
			svg.clear(false);

			g = svg.group({stroke: 'black', 'stroke-width': 2});
			div.data("g", g);
		    }

//		    alert(x1 + " " + x2 + " " + y1 + " " + y2);
                    svg.line(g, x1, 100 - y1*mult_c, x2, 100 - y2*mult_c);
		    		    
		    div.data('last_x',x2);
		    div.data('last_y',y2);		    
		});
            },"xml");
        } else {}
    });
//});


function random(range) {
	return Math.floor(Math.random() * range);
}
