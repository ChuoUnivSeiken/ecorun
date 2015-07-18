/// <reference path="./typings/tsd.d.ts"/>

import $ = require('jquery');
import events = require('events');
import car_visualize = require("./car_visualize");

export class AppConnection extends events.EventEmitter {
    socketio = null;
    private static _instance: AppConnection = null;
    serial_states: Object;
    constructor() {
        super();
        if (AppConnection._instance) {
            throw new Error("must use the getInstance.");
        }
        AppConnection._instance = this;
    }

    static get_instance(): AppConnection {
        if (AppConnection._instance == null) {
            AppConnection._instance = new AppConnection();
            AppConnection._instance.initialize();
        }
        return AppConnection._instance;
    }

    initialize() {
        var host = window.location.host;
        var io = require('socket.io-client');
        this.socketio = io.connect('http://' + host + ':80');

        this.socketio.json.on('serial_ports', (data) => {
            var arr = data.value;
            if (arr.length > 0) {
                this.emit('serial_ports', arr);
            }
        });

        this.socketio.json.on('serial_connected', (data) => {
            this.emit('serial_connected', data);
        });

        this.socketio.json.on('serial_disconnected', (data) => {
            this.emit('serial_disconnected', data);
        });

        this.socketio.json.on('data', (data) => {
            this.emit('data', data);
        });

        var id = Math.floor(Math.random() * 100);
        this.socketio.emit('connected', id);
    }

    update_serial_port_state() {
        this.socketio.emit('serial_list_ports', {});
    }

    write_serial(message: string) {
        this.socketio.emit('serial_write', {
            message: message
        });
    }

    request_data(id: string) {
        this.socketio.emit('request_data', {
            id: id
        });
    }

    save_data(id: string, data: Object) {
        this.socketio.emit('save_data', {
            id: id,
            data: data
        });
    }

    send_data(id: string, data: Object, size: number) {
        this.socketio.emit('send_data', {
            id: id,
            data: data,
            size: size
        });
    }

    start_connection() {
        var portName = $('[name=port]').val();
        var bitRate = parseInt($('[name=baudrate]').val());

        this.socketio.emit('serial_connect', {
            portName: portName,
            bitRate: bitRate
        });
    }

    end_connection() {
        this.socketio.emit('serial_disconnect', {});
    }
}

export function set_connection_state(state: string) {
    var connection = AppConnection.get_instance();
    var end_connection = AppConnection.get_instance().end_connection;
    var start_connection = AppConnection.get_instance().start_connection;
    if (state == 'connect') {
        $('#connection_state').html('接続中');
        $('#port').attr('disabled', 'disabled');
        $('#baudrate').attr('disabled', 'disabled');
        $('#connect_start_stop').off('click');
        $('#connect_start_stop').on('click', () => { connection.end_connection(); });
        $('#connect_start_stop').val('シリアル通信終了');
    } else {
        $('#connection_state').html('未接続');
        $('#port').removeAttr('disabled');
        $('#baudrate').removeAttr('disabled');
        $('#connect_start_stop').off('click');
        $('#connect_start_stop').on('click', () => { connection.start_connection(); });
        $('#connect_start_stop').val('シリアル通信開始');
    }
}

export function initialize() {
    var connection = AppConnection.get_instance();

    connection.on('serial_ports', (arr) => {
        $('#port').html(
            arr.map((item) => {
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

    connection.on('serial_connected', (data) => {
        set_connection_state('connect')
    });

    connection.on('serial_disconnected', (data) => {
        set_connection_state('disconnect')
    });

    var data_received = function(data) {
        car_visualize.car_add_data(data);
    };

    connection.on('data', data_received);

    $('#list').click(() => {
        connection.update_serial_port_state();
    });

    $('#port').change(() => {
    });

    connection.update_serial_port_state();
}
