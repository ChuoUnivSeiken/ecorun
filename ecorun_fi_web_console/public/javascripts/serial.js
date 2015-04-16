$(function() {
	var socketio = io.connect('http://192.168.1.104:80');

	socketio.json.on('serial_ports', function(data) {
		var arr = data.value;
		$('#port').html(
			arr.map(function(item) {
				var portName = item.comName;
				return '<option value="' + portName + '">' + portName + '</option>';
			}));
		var item = arr[0];
		if (item.state == 'connect') {
			$('#connection_state').html('接続中');
			$('#connect_start_stop').off('click', startConnection);
			$('#connect_start_stop').off('click', endConnection);
			$('#connect_start_stop').on('click', endConnection);
			$('#connect_start_stop').val('シリアル通信終了');
		} else {
			$('#connection_state').html('未接続');
			$('#connect_start_stop').off('click', endConnection);
			$('#connect_start_stop').off('click', startConnection);
			$('#connect_start_stop').on('click', startConnection);
			$('#connect_start_stop').val('シリアル通信開始');
		}
	});

	socketio.json.on('serial_connected', function(data) {
		$('#connection_state').html('接続中');
		$('#port').attr('disabled', 'disabled');
		$('#baudrate').attr('disabled', 'disabled');
		$('#connect_start_stop').off('click', startConnection);
		$('#connect_start_stop').on('click', endConnection);
		$('#connect_start_stop').val('シリアル通信終了');
	});

	socketio.json.on('serial_connect', function(data) {});

	socketio.json.on('serial_disconnected', function(data) {
		$('#connection_state').html('未接続');
		$('#port').removeAttr('disabled');
		$('#baudrate').removeAttr('disabled');
		$('#connect_start_stop').off('click', endConnection);
		$('#connect_start_stop').on('click', startConnection);
		$('#connect_start_stop').val('シリアル通信開始');
	});

	socketio.json.on('serial_disconnect', function(data) {});

	socketio.json.on('data', function(data) {
		data_received(data);
	});

	var startConnection = function() {
		var portName = $('[name=port]').val();
		var bitRate = parseInt($('[name=baudrate]').val());

		socketio.emit('serial_connect', {
			portName: portName,
			bitRate: bitRate
		});
	};

	var endConnection = function() {
		socketio.emit('serial_disconnect', {});
	};
	$('#connect_start_stop').on('click', startConnection);

	$('#list').click(function() {
		socketio.emit('serial_list_ports', {});
	});

	var serialWrite = function(message) {
		socketio.emit('serial_write', {
			message: message
		});
	};

	var id = Math.floor(Math.random() * 100);
	socketio.emit('connected', id);
	socketio.emit('serial_list_ports');
});
