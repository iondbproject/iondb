$( document ).ready(function() {
	var fragment = $(".code-snippet:not(.contents > .code-snippet)");
	var header = $('<h3 class="code-snippet-header">Source Code</h3>');
	var accordion = $('<div class="accordion"></div>');
	fragment.wrap(accordion);
	$('.accordion').prepend(header);
	$('.accordion').accordion({
		collapsible: true,
		active: false,
		icons: {"header": "ui-icon-plus", "activeHeader": "ui-icon-minus"}
	});

	// // $('.table td, .table tr, .table th').has('groupheader').css('border-top', 'none');

	// // var svg_buttons = '<div class="buttons">' +
	// // 	'<button class="zoom-in">Zoom In</button>' +
	// // 	'<button class="zoom-out">Zoom Out</button>' +
	// // 	'<input type="range" class="zoom-range">' +
	// // 	'<button class="reset">Reset</button></div>';

	var iframe = $('.dyncontent iframe');

	iframe.each(function() {
		content = $(this).contents().find('svg');
		$(this).before('<a href="' + content.attr('src') + '"><span class="glyphicon glyphicon-file" aria-hidden="true"></span></a>');
		$(this).hide();
	});

	$(window).resize(function() {
		$(window).width();
	});

	// if (is_cross_site_safe) {
	// 	iframe.remove();

	// 	// var svg = $('svg');
	//  //    svg.parent().panzoom({
	//  //        $zoomIn: svg.find(".zoom-in"),
	//  //        $zoomOut: svg.find(".zoom-out"),
	//  //        $zoomRange: svg.find(".zoom-range"),
	//  //        $reset: svg.find(".reset"),
	//  //        startTransform: 'scale(0.9)',
	//  //        maxScale: 20,
	//  //        increment: 0.1,
	//  //        contain: true
 //  //   	}).panzoom('zoom', true);
	// }
	// else {
	// 	iframe.width('100%');
	// }
});