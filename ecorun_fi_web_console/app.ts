/// <reference path="typings/tsd.d.ts" />
/// <reference path="./carcomm.ts" />

var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');

var routes = require('./routes/index');
var users = require('./routes/users');

var app = express();

// view engine setup
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');

// uncomment after placing your favicon in /public
//app.use(favicon(__dirname + '/public/favicon.ico'));
app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(require('stylus').middleware(path.join(__dirname, 'public')));
app.use(express.static(path.join(__dirname, 'public')));

app.use('/', routes);
app.use('/users', users);

interface Error {
    status?: number;
};

// catch 404 and forward to error handler
app.use(function (req, res, next) {
    var err = new Error('Not Found');
    err['status'] = 404;
    next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
    app.use(function (err, req, res, next) {
        res.status(err.status || 500);
        res.render('error', {
            message: err.message,
            error: err
        });
    });
}

// production error handler
// no stacktraces leaked to user
app.use(function (err, req, res, next) {
    res.status(err.status || 500);
    res.render('error', {
        message: err.message,
        error: {}
    });
});

var server = app.listen(80, function () {
    var host = server.address().address;
    var port = server.address().port;
    console.log('listening at http://%s:%s', host, port);
});

var io = require('socket.io').listen(server);

var serial = require('serialport');
var _ = require('lodash');
var adler32 = require('adler-32');

var idHash = {};
var serialport = null;
var serialStates = {};

var uart_buf = [];

import carcomm = require("./carcomm");
var date = new Date();

var uart_message_received = function (msg) {
    var line = msg.split(' ');

    if (line === null || line.length === 0) {
        console.log('Invalid msg :', msg);
        return;
    }

    var command = line[0];
    var obj;
    
    switch (command) {
        case 'msg':
            if (line.length < 2) {
                console.log('Invalid parameter length :', msg);
                return;
            }
            var message = line.slice(1).join(' ').replace("<", "").replace(">", "");
            console.log(message);
            break;
        case 'put':
            if (line.length !==7) {
                console.log('Invalid parameter length :', msg);
                return;
            }

            var id = line[1];
            var data_size = parseInt(line[2]);
            var data = line[3];
            var sum = parseInt(line[4]);
            var seconds = parseInt(line[5]);
            var counter = parseInt(line[6]);
            console.log(seconds);
            
            var buf = new Buffer(data, 'base64');
            
            if (buf.length != data_size) {
                console.log('Invalid data size :', msg);
            }
            
            var stream = new carcomm.StructReader(buf);
            
            switch (id) {
                case 'engine_data':
                    
                    console.log('date : ', new Date().getSeconds() - date.getSeconds())
                    console.log('receive : ', id);
                    obj = stream
                    .readUInt32('rev')
                    .readUInt32('is_fuel_cut')
                    .readUInt32('is_af_rich')
                    .readUInt32('th')
                    .readUInt32('oil_temp')
                    .readUInt32('current_total_injected_time')
                    .readUInt32('current_inject_started_count')
                    .readUInt32('current_inject_ended_count')
                    .structs();
                    break;
                case 'car_data':
                    console.log('receive : ' + id);
                    obj = stream
                    .readUInt32('vattery_voltage')
                    .readUInt32('wheel_count')
                    .readUInt32('wheel_rotation_period')
                    .structs();
                    break;
            }
            var check_sum = adler32.buf(buf);
            if (check_sum === sum && !(obj === null || obj === undefined)) {
                io.sockets.emit('data', {
                    id: id,
                    value: obj
                });
            }
            break;
        default:
            console.log('Invalid command : ', command);
            break;
    }
};

var uart_received = function (data) {
    switch (data) {
        case 0x03:
            uart_message_received(_.reduce(uart_buf, function (result, ch) {
                return result + String.fromCharCode(ch);
            }, ""));
            break;
        case 0x02:
            uart_buf = [];
            break;
        default:
            uart_buf.push(data);
            break;
    }
};

var serial_write = function (data) {
    var errfunc = function (error, results) {
        if (error !== undefined && error !== null) {
            console.log('serial port writing error : ' + error);
        }
    };
    
    var stx = String.fromCharCode(0x02);
    var etx = String.fromCharCode(0x03);
    serialport.write(stx + data + etx, errfunc);
};



setInterval(function () {
    if (serialport !== null) {/*
        var text = new Buffer(32);
        for (var i = 0; i < text.length; i += 4) {
            text.writeUInt32LE(i, i);
        }
        var str = text.toString('base64');
        var check_sum = adler32.buf(text);
        check_sum = 0;
        var str2 = check_sum.toString(16);
        serial_write('put engine_data ' + text.length.toString() + ' ' + str + ' ' + check_sum.toString(10));*/
        
        serial_write('get car_data');
        date = new Date()
        console.log('send : ', date);
        serial_write('get engine_data');
    }
}, 100);

io.sockets.on('connection', function (socket) {
    socket.on('connected', function (id) {
        idHash[socket.id] = id;
        console.log('login : ' + id);
    });
    
    socket.on('serial_list_ports', function () {
        serial.list(function (err, ports) {
            var arr = _.map(ports, function (port) {
                return {
                    comName: port.comName,
                    pnpId: port.pnpId,
                    manufacturer: port.manufacturer,
                    state: serialStates[port.comName]
                };
            });
            io.sockets.emit('serial_ports', {
                value: arr
            });
        });
    });
    
    socket.on('serial_connect', function (connection_data) {
        var _serialport = new serial.SerialPort(connection_data.portName, {
            baudrate: connection_data.bitRate,
            dataBits: 8,
            parity: 'none',
            stopBits: 1,
            flowControl: false
        }, function (error) {
            if (error !== undefined && error !== null) {
                console.log('serial port opening error : ' + error);
            }
        });
        _serialport.on('open', function () {
            serialport = _serialport;
            console.log(connection_data.portName + ' opened.');
            serialport.on('data', function (data) {
                _.forEach(data, function (item) {
                    uart_received(item);
                });
            });
            serialport.on('close', function () {
                console.log(connection_data.portName + ' closed.');
                serialStates[connection_data.portName] = undefined;
                io.sockets.emit('serial_disconnected', {});
            });
            serialStates[connection_data.portName] = 'connect';
            io.sockets.emit('serial_connected', {});
        });
    });
    
    socket.on('serial_disconnect', function () {
        var _serialport = serialport;
        serialport = null;
        _serialport.close(function (error) {
            if (error !== undefined && error !== null) {
                console.log('serial port closing error : ' + error);
            }
        });
    });
    
    socket.on('serial_write', function (data) {
        serial_write(data.message);
    });
    
    socket.on('disconnect', function () {
        var id = idHash[socket.id];
        if (idHash[socket.id]) {
            delete idHash[socket.id];
            console.log('logout : ' + id);
        }
    });
});

module.exports = app;
