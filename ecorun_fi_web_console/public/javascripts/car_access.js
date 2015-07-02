/// <reference path="./typings/tsd.d.ts"/>
var __extends = (this && this.__extends) || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
};
var $ = require('jquery');
var events = require('events');
var AppConnection = (function (_super) {
    __extends(AppConnection, _super);
    function AppConnection() {
        _super.call(this);
        this.socketio = null;
        if (AppConnection._instance) {
            throw new Error("must use the getInstance.");
        }
        AppConnection._instance = this;
    }
    AppConnection.get_instance = function () {
        if (AppConnection._instance == null) {
            AppConnection._instance = new AppConnection();
            AppConnection._instance.initialize();
        }
        return AppConnection._instance;
    };
    AppConnection.prototype.initialize = function () {
        var _this = this;
        var host = window.location.host;
        var io = require('socket.io-client');
        this.socketio = io.connect('http://' + host + ':80');
        this.socketio.json.on('serial_ports', function (data) {
            var arr = data.value;
            if (arr.length > 0) {
                _this.emit('serial_ports', arr);
            }
        });
        this.socketio.json.on('serial_connected', function (data) {
            _this.emit('serial_connected', data);
        });
        this.socketio.json.on('serial_disconnected', function (data) {
            _this.emit('serial_disconnected', data);
        });
        this.socketio.json.on('data', function (data) {
            _this.emit('data', data);
        });
        var id = Math.floor(Math.random() * 100);
        this.socketio.emit('connected', id);
    };
    AppConnection.prototype.update_serial_port_state = function () {
        this.socketio.emit('serial_list_ports', {});
    };
    AppConnection.prototype.write_serial = function (message) {
        this.socketio.emit('serial_write', {
            message: message
        });
    };
    AppConnection.prototype.request_data = function (id) {
        this.socketio.emit('request_data', {
            id: id
        });
    };
    AppConnection.prototype.send_data = function (id, data, size) {
        this.socketio.emit('send_data', {
            id: id,
            data: data,
            size: size
        });
    };
    AppConnection.prototype.start_connection = function () {
        var portName = $('[name=port]').val();
        var bitRate = parseInt($('[name=baudrate]').val());
        this.socketio.emit('serial_connect', {
            portName: portName,
            bitRate: bitRate
        });
    };
    AppConnection.prototype.end_connection = function () {
        this.socketio.emit('serial_disconnect', {});
    };
    AppConnection._instance = null;
    return AppConnection;
})(events.EventEmitter);
exports.AppConnection = AppConnection;
function set_connection_state(state) {
    var connection = AppConnection.get_instance();
    var end_connection = AppConnection.get_instance().end_connection;
    var start_connection = AppConnection.get_instance().start_connection;
    if (state == 'connect') {
        $('#connection_state').html('接続中');
        $('#port').attr('disabled', 'disabled');
        $('#baudrate').attr('disabled', 'disabled');
        $('#connect_start_stop').off('click');
        $('#connect_start_stop').on('click', function () { connection.end_connection(); });
        $('#connect_start_stop').val('シリアル通信終了');
    }
    else {
        $('#connection_state').html('未接続');
        $('#port').removeAttr('disabled');
        $('#baudrate').removeAttr('disabled');
        $('#connect_start_stop').off('click');
        $('#connect_start_stop').on('click', function () { connection.start_connection(); });
        $('#connect_start_stop').val('シリアル通信開始');
    }
}
exports.set_connection_state = set_connection_state;
function initialize() {
    var car_visualize = require("./car_visualize.js");
    var connection = AppConnection.get_instance();
    connection.on('serial_ports', function (arr) {
        $('#port').html(arr.map(function (item) {
            var portName = item.comName;
            return '<option value="' + portName + '">' + portName + '</option>';
        }));
        var connected = arr[0];
        for (var i in arr) {
            var item = arr[i];
            if (item.state == 'connect') {
                connected = item;
                $("#port").val(item.comName);
                break;
            }
        }
        set_connection_state(connected.state);
    });
    connection.on('serial_connected', function (data) {
        set_connection_state('connect');
    });
    connection.on('serial_disconnected', function (data) {
        set_connection_state('disconnect');
    });
    var data_received = function (data) {
        car_visualize.car_add_data(data);
    };
    connection.on('data', data_received);
    $('#list').click(function () {
        connection.update_serial_port_state();
    });
    $('#port').change(function () {
    });
    connection.update_serial_port_state();
}
exports.initialize = initialize;
