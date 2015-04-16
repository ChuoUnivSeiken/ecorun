$(function() {
	var changeTab = function(tabname) {
		$('#tab1_content').css("display", "none");
		$('#tab2_content').css("display", "none");
		$('#tab3_content').css("display", "none");
		$('#tab4_content').css("display", "none");
		$('#tab5_content').css("display", "none");
		// 指定箇所のみ表示
		$("#" + tabname).css("display", "block");
	};

	changeTab('tab1_content');

	$('#tab1').click(function() {
		//if ($("#connection_state").html() == "接続中") {
		changeTab('tab1_content');
		//}
	});

	$('#tab2').click(function() {
		//if ($("#connection_state").html() == "接続中") {
		changeTab('tab2_content');
		//}
	});

	$('#tab3').click(function() {
		//if ($("#connection_state").html() == "接続中") {
			changeTab('tab3_content');
		//}
	});

	$('#tab4').click(function() {
		if ($("#connection_state").html() == "接続中") {
			changeTab('tab4_content');
		}
	});

	$('#tab5').click(function() {
		//if ($("#connection_state").html() == "接続中") {
			changeTab('tab5_content');
		//}
	});
});