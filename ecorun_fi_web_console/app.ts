/// <reference path="typings/tsd.d.ts" />

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

// catch 404 and forward to error handler
app.use(function(req, res, next) {
    var err = new Error('Not Found');
    err['status'] = 404;
    next(err);
});

// error handlers

// development error handler
// will print stacktrace
if (app.get('env') === 'development') {
    app.use(function(err, req, res, next) {
        res.status(err.status || 500);
        res.render('error', {
            message: err.message,
            error: err
        });
    });
}

// production error handler
// no stacktraces leaked to user
app.use(function(err, req, res, next) {
    res.status(err.status || 500);
    res.render('error', {
        message: err.message,
        error: {}
    });
});

var server = app.listen(80, function() {
    var host = server.address().address;
    var port = server.address().port;
    console.log('listening at http://%s:%s', host, port);
});

import socketio = require('socket.io');
var io = socketio.listen(server);
import _ = require('lodash');

var idHash = {};

import carcomm = require('./car_comm');
import carserialize = require("./car_serialize");
carserialize.CarDatabase.connect();
var carTransmitter: carcomm.CarTransmitter;

setInterval(() => {
    if (carTransmitter != null) {
        carTransmitter.requestData('car_data');
        carTransmitter.requestData('engine_data');
    }
}, 100);
setInterval(() => {
    if (carTransmitter != null) {
        carTransmitter.requestData('basic_inject_time_map');
    }
}, 1000);
io.sockets.on('connection', function(socket) {
    socket.on('connected', (id) => {
        idHash[socket.id] = id;
        console.log('login : ' + id);
    });

    socket.on('serial_list_ports', () => {
        carcomm.CarSerialPort.getPortInfo((arr) => {
            io.sockets.emit('serial_ports', {
                value: arr
            });
        });
    });

    socket.on('serial_connect', (connection_data) => {
        carTransmitter = new carcomm.CarTransmitter(connection_data.portName, connection_data.bitRate);

        carTransmitter.addOpenedHandler((e) => {
            carTransmitter.addDataReceivedHandler((e) => {
                var data = e.value;
                io.sockets.emit('data', data);
                data.value['timestamp'] = new Date();
                carserialize.CarDatabase.saveData(data.id, data.value)
            });
            carTransmitter.addMessageReceivedHandler((e) => {
                io.sockets.emit('msg', e.value);
            });
            io.sockets.emit('serial_connected', {});
        });
        carTransmitter.addClosedHandler((e) => {
            io.sockets.emit('serial_disconnected', {});
        });
        carTransmitter.open();
    });

    socket.on('serial_disconnect', () => {
        var _carSerialport = carTransmitter;
        carTransmitter = null;
        _carSerialport.close();
    });

    socket.on('serial_write', (data) => {
        if (carTransmitter != null && carTransmitter.isOpening) {
            carTransmitter.write(data.message);
        }
    });

    socket.on('disconnect', () => {
        var id = idHash[socket.id];
        if (idHash[socket.id]) {
            delete idHash[socket.id];
            console.log('logout : ' + id);
        }
    });
});

module.exports = app;
