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

	$(window).resize(function() {
		$(window).width();
	});
});