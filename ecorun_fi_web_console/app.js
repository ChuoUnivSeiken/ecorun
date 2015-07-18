/// <reference path="typings/tsd.d.ts" />
var express = require('express');
var socketio = require('socket.io');
var path = require('path');
var carcomm = require('./car_comm');
var carserialize = require("./car_serialize");
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var routes = require('./routes/index');
var users = require('./routes/users');
var app = express();
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');
app.use(logger('dev'));
app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(require('stylus').middleware(path.join(__dirname, 'public')));
app.use(express.static(path.join(__dirname, 'public')));
app.use('/', routes);
app.use('/users', users);
app.use(function (req, res, next) {
    var err = new Error('Not Found');
    err['status'] = 404;
    next(err);
});
if (app.get('env') === 'development') {
    app.use(function (err, req, res, next) {
        res.status(err.status || 500);
        res.render('error', {
            message: err.message,
            error: err
        });
    });
}
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
var io = socketio.listen(server);
var idHash = {};
carserialize.CarDatabase.connect();
var carTransmitter;
var timer = setInterval(function () {
    if (carTransmitter != null) {
        carTransmitter.requestData('car_data');
        carTransmitter.requestData('engine_data');
    }
}, 100);
io.sockets.on('connection', function (socket) {
    socket.on('connected', function (id) {
        idHash[socket.id] = id;
        console.log('login : ' + id);
    });
    socket.on('serial_list_ports', function () {
        carcomm.CarSerialPort.getPortInfo(function (arr) {
            io.sockets.emit('serial_ports', {
                value: arr
            });
        });
    });
    socket.on('serial_connect', function (connection_data) {
        carTransmitter = new carcomm.CarTransmitter(connection_data.portName, connection_data.bitRate);
        carTransmitter.on('opened', function () {
            io.sockets.emit('serial_connected', {});
        });
        carTransmitter.on('closed', function () {
            io.sockets.emit('serial_disconnected', {});
        });
        carTransmitter.on('obj', function (data) {
            io.sockets.emit('data', data);
            data.value['timestamp'] = new Date();
            carserialize.CarDatabase.saveData(data.id, data.value);
        });
        carTransmitter.on('msg', function (msg) {
            io.sockets.emit('msg', msg);
            console.log(msg);
        });
        carTransmitter.open();
    });
    socket.on('serial_disconnect', function () {
        var _carSerialport = carTransmitter;
        carTransmitter = null;
        _carSerialport.close();
    });
    socket.on('serial_write', function (data) {
        if (carTransmitter != null && carTransmitter.isOpening) {
            carTransmitter.write(data.message);
        }
    });
    socket.on('request_data', function (data) {
        if (carTransmitter != null && carTransmitter.isOpening) {
            carTransmitter.requestData(data.id);
        }
    });
    socket.on('send_data', function (data) {
        if (carTransmitter != null && carTransmitter.isOpening) {
            carTransmitter.sendData(data.id, data.data);
        }
    });
    socket.on('save_data', function (data) {
        if (carTransmitter != null && carTransmitter.isOpening) {
            carTransmitter.write("exec save-settings");
        }
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
