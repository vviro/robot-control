 /*
 * [ 0.2 ] Original by Jonathan Howard
 * [ 0.3 ] Updated for current jQuery releases,
 *         and formatted for jQuery namespace by Charles Phillips
 *         <charles@doublerebel.com>
 *
 * jQuery Pause
 * version 0.3
 *
 * Requires: jQuery 1.2 (I think that's when jQuery updated the queue method)
 *
 * Feel free to do whatever you'd like with this, just please give credit where
 * credit is due.
 *
 *
 *
 * pause() will hold everything in the queue for a given number of milliseconds,
 * or 1000 milliseconds if none is given.
 *
 *
 *
 * unpause() will clear the queue of everything of a given type, or 'fx' if no
 * type is given.
 */

(function($) {
	$.fn.extend({
		pause: function(milli,type) {
			milli = milli || 1000;
			type = type || "fx";
			return this.queue(type,function(){
				var self = this;
				setTimeout(function(){
					$(self).dequeue();
				},milli);
			});
		},
		clearQueue: function(type) {
			return this.each(function(){
				type = type || "fx";
				if(this.queue && this.queue[type]) {
					this.queue[type].length = 0;
				}
			});
		},
		unpause: $.fn.clearQueue
	});
})(jQuery);