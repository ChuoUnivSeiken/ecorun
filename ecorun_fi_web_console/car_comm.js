/// <reference path="typings/tsd.d.ts" />
var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};
var _ = require('lodash');
var serialport = require('serialport');
var adler32 = require('adler-32');
var events = require('events');
var CarSerialPort = (function (_super) {
    __extends(CarSerialPort, _super);
    function CarSerialPort(portName, bitRate) {
        _super.call(this);
        this.portName = portName;
        this.bitRate = bitRate;
        this.serialport = null;
        this.byteBuffer = [];
    }
    Object.defineProperty(CarSerialPort.prototype, "isOpening", {
        get: function () {
            return this.serialport != null;
        },
        enumerable: true,
        configurable: true
    });
    CarSerialPort.prototype.byteReceived = function (byte) {
        switch (byte) {
            case 0x03:
                this.dataReceived(_.reduce(this.byteBuffer, function (result, ch) { return result + String.fromCharCode(ch); }, ""));
                break;
            case 0x02:
                this.byteBuffer = [];
                break;
            default:
                this.byteBuffer.push(byte);
                break;
        }
    };
    CarSerialPort.prototype.dataReceived = function (data) {
        this.emit('data', data);
    };
    CarSerialPort.prototype.open = function () {
        var _this = this;
        var _serialport = new serialport.SerialPort(this.portName, {
            baudrate: this.bitRate,
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
            CarSerialPort.serialStates[_this.portName] = 'connect';
            console.log(_this.portName + ' opened.');
            _this.serialport = _serialport;
            _this.serialport.on('data', function (data) {
                _.forEach(data, function (byte) { return _this.byteReceived(byte); });
            });
            _this.serialport.on('close', function () {
                CarSerialPort.serialStates[_this.portName] = undefined;
                console.log(_this.portName + ' closed.');
                _this.emit('closed');
            });
            _this.emit('opened');
        });
    };
    CarSerialPort.prototype.close = function () {
        if (this.isOpening) {
            var _serialport = this.serialport;
            this.serialport = null;
            _serialport.close(function (error) {
                if (error !== undefined && error !== null) {
                    console.log('Serial port closing error : ' + error);
                }
            });
        }
    };
    CarSerialPort.prototype.write = function (str) {
        if (!this.isOpening) {
            throw new Error("This port is not opening.");
        }
        var errfunc = function (error, results) {
            if (error !== undefined && error !== null) {
                console.log('Serial port writing error : ' + error);
            }
        };
        var stx = String.fromCharCode(0x02);
        var etx = String.fromCharCode(0x03);
        this.serialport.write(stx + str + etx, errfunc);
    };
    CarSerialPort.getPortInfo = function (callback) {
        serialport.list(function (err, ports) {
            callback(_.map(ports, function (port) {
                var state = CarSerialPort.serialStates[port.comName];
                console.log(state == undefined ? 'disconnect' : state);
                return {
                    comName: port.comName,
                    pnpId: port.pnpId,
                    manufacturer: port.manufacturer,
                    state: state == undefined ? 'disconnect' : state
                };
            }));
        });
    };
    CarSerialPort.serialStates = {};
    return CarSerialPort;
})(events.EventEmitter);
exports.CarSerialPort = CarSerialPort;
var CarTransmitter = (function (_super) {
    __extends(CarTransmitter, _super);
    function CarTransmitter(portName, bitRate) {
        _super.call(this, portName, bitRate);
    }
    CarTransmitter.prototype.dataReceived = function (data) {
        _super.prototype.dataReceived.call(this, data);
        var received = CarTransmitter.parseCommandLine(data);
        if (received != undefined && received.cmd != undefined) {
            if (received.cmd == 'put') {
                this.emit('obj', received.data);
            }
            else if (received.cmd == 'msg') {
                this.emit('msg', received.message);
            }
        }
    };
    CarTransmitter.toByteBuffer = function (buffer) {
        var ab = new ArrayBuffer(buffer.length);
        var view = new Uint8Array(ab);
        for (var i = 0; i < buffer.length; ++i) {
            view[i] = buffer[i];
        }
        return ab;
    };
    CarTransmitter.prototype.requestData = function (id) {
        if (this.isOpening) {
            this.write('get ' + id);
        }
    };
    CarTransmitter.prototype.sendData = function (id, data) {
        if (this.isOpening) {
            switch (id) {
                case 'basic_inject_time_map':
                    var buffer = new Buffer(data, 'base64');
                    var command = "put " + id + " " + buffer.length.toString() + " " + data + " " + (adler32.buf(buffer) >>> 0);
                    console.log(command);
                    this.write(command);
                    break;
            }
        }
    };
    CarTransmitter.parseCommandLine = function (line) {
        var tokens = line.split(' ');
        if (tokens === null || tokens.length === 0) {
            console.log('Invalid tokens :', tokens);
            return null;
        }
        var command = tokens[0];
        switch (command) {
            case 'msg':
                if (tokens.length < 2) {
                    console.log('Invalid parameter length :', line);
                    return;
                }
                var message = tokens.slice(1).join(' ').replace("<", "").replace(">", "");
                return {
                    cmd: 'msg',
                    message: message
                };
                break;
            case 'put':
                if (tokens.length !== 7) {
                    console.log('Invalid parameter length :', line);
                    return;
                }
                var id = tokens[1];
                var data_size = parseInt(tokens[2]);
                var data = tokens[3];
                var sum = parseInt(tokens[4]);
                var sec = parseInt(tokens[5]);
                var microsec = parseInt(tokens[6]);
                var buf = new Buffer(data, 'base64');
                if (buf.length != data_size) {
                    console.log('Invalid data size :', line);
                }
                var stream = new ObjectReader(buf);
                var obj = null;
                switch (id) {
                    case 'engine_data':
                        obj = stream.readUInt32('rev').readUInt32('is_fuel_cut').readUInt32('is_af_rich').readUInt32('th').readUInt32('oil_temp').readUInt32('intake_temp').readUInt32('vacuum').readUInt32('current_total_injected_time').readUInt32('current_inject_started_count').readUInt32('current_inject_ended_count').toObject();
                        break;
                    case 'car_data':
                        obj = stream.readUInt32('vattery_voltage').readUInt32('wheel_count').readUInt32('wheel_rotation_period').toObject();
                        break;
                    case 'basic_inject_time_map':
                        obj = {
                            basic_inject_time_map: data
                        };
                        break;
                }
                var check_sum = adler32.buf(buf) >>> 0;
                if (check_sum === sum && !(obj === null || obj === undefined)) {
                    return {
                        cmd: 'put',
                        data: {
                            id: id,
                            value: obj,
                            timestamp: {
                                microsec: microsec,
                                sec: sec
                            }
                        }
                    };
                }
                break;
            default:
                console.log('Invalid command : ', command);
                return null;
                break;
        }
    };
    return CarTransmitter;
})(CarSerialPort);
exports.CarTransmitter = CarTransmitter;
var ObjectReader = (function () {
    function ObjectReader(buffer, offset, values, names) {
        if (offset === void 0) { offset = 0; }
        if (values === void 0) { values = []; }
        if (names === void 0) { names = []; }
        this.buffer = buffer;
        this.offset = offset;
        this.values = values;
        this.names = names;
    }
    ObjectReader.prototype.readInt8 = function (name) {
        if (this.offset + 1 > this.buffer.length) {
            return new ObjectReaderNone();
        }
        return new ObjectReader(this.buffer, this.offset + 1, this.values.concat(this.buffer.readInt8(this.offset)), this.names.concat(name));
    };
    ObjectReader.prototype.readUInt32 = function (name) {
        if (this.offset + 4 > this.buffer.length) {
            return new ObjectReaderNone();
        }
        return new ObjectReader(this.buffer, this.offset + 4, this.values.concat(this.buffer.readUInt32LE(this.offset)), this.names.concat(name));
    };
    ObjectReader.prototype.readInt16 = function (name) {
        if (this.offset + 2 > this.buffer.length) {
            return new ObjectReaderNone();
        }
        return new ObjectReader(this.buffer, this.offset + 2, this.values.concat(this.buffer.readInt16LE(this.offset)), this.names.concat(name));
    };
    ObjectReader.prototype.toObject = function () {
        return _.object(this.names, this.values);
    };
    return ObjectReader;
})();
exports.ObjectReader = ObjectReader;
var ObjectReaderNone = (function (_super) {
    __extends(ObjectReaderNone, _super);
    function ObjectReaderNone() {
        _super.call(this, null);
    }
    ObjectReaderNone.prototype.readInt8 = function (name) {
        return new ObjectReaderNone();
    };
    ObjectReaderNone.prototype.readUInt32 = function (name) {
        return new ObjectReaderNone();
    };
    ObjectReaderNone.prototype.readInt16 = function (name) {
        return new ObjectReaderNone();
    };
    return ObjectReaderNone;
})(ObjectReader);
exports.ObjectReaderNone = ObjectReaderNone;
