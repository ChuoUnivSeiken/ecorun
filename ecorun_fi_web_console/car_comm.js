/// <reference path="typings/tsd.d.ts" />
var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};
var _ = require('lodash');
var adler32 = require('adler-32');
var serialport = require('serialport');
var EventDispatcher = (function () {
    function EventDispatcher() {
        this.listeners = {};
    }
    EventDispatcher.prototype.dispatchEvent = function (event) {
        var e;
        var type;
        if (event instanceof Event) {
            type = event.type;
            e = event;
        }
        else {
            type = event;
            e = new Event(type);
        }
        if (this.listeners[type] != null) {
            e.currentTarget = this;
            for (var i = 0; i < this.listeners[type].length; i++) {
                var listener = this.listeners[type][i];
                try {
                    listener.handler(e);
                }
                catch (error) {
                    console.error(error.stack);
                }
            }
        }
    };
    EventDispatcher.prototype.addEventListener = function (type, callback, priolity) {
        if (priolity === void 0) { priolity = 0; }
        if (this.listeners[type] == null) {
            this.listeners[type] = [];
        }
        this.listeners[type].push(new EventListener(type, callback, priolity));
        this.listeners[type].sort(function (listener1, listener2) {
            return listener2.priolity - listener1.priolity;
        });
    };
    EventDispatcher.prototype.removeEventListener = function (type, callback) {
        if (this.hasEventListener(type, callback)) {
            for (var i = 0; i < this.listeners[type].length; i++) {
                var listener = this.listeners[type][i];
                if (listener.equalCurrentListener(type, callback)) {
                    listener.handler = null;
                    this.listeners[type].splice(i, 1);
                    return;
                }
            }
        }
    };
    EventDispatcher.prototype.clearEventListener = function () {
        this.listeners = {};
    };
    EventDispatcher.prototype.containEventListener = function (type) {
        if (this.listeners[type] == null)
            return false;
        return this.listeners[type].length > 0;
    };
    EventDispatcher.prototype.hasEventListener = function (type, callback) {
        if (this.listeners[type] == null)
            return false;
        for (var i = 0; i < this.listeners[type].length; i++) {
            var listener = this.listeners[type][i];
            if (listener.equalCurrentListener(type, callback)) {
                return true;
            }
        }
        return false;
    };
    return EventDispatcher;
})();
exports.EventDispatcher = EventDispatcher;
var EventListener = (function () {
    function EventListener(type, handler, priolity) {
        if (type === void 0) { type = null; }
        if (handler === void 0) { handler = null; }
        if (priolity === void 0) { priolity = 0; }
        this.type = type;
        this.handler = handler;
        this.priolity = priolity;
    }
    EventListener.prototype.equalCurrentListener = function (type, handler) {
        if (this.type == type && this.handler == handler) {
            return true;
        }
        return false;
    };
    return EventListener;
})();
var Event = (function () {
    function Event(type, value) {
        if (type === void 0) { type = null; }
        if (value === void 0) { value = null; }
        this.type = type;
        this.value = value;
    }
    Event.COMPLETE = "complete";
    Event.CHANGE_PROPERTY = "changeProperty";
    return Event;
})();
exports.Event = Event;
var CarSerialPort = (function () {
    function CarSerialPort(portName, bitRate) {
        this.portName = portName;
        this.bitRate = bitRate;
        this.eventDispatcher = new EventDispatcher();
        this.serialport = null;
    }
    Object.defineProperty(CarSerialPort.prototype, "isOpening", {
        get: function () {
            return this.serialport != null;
        },
        enumerable: true,
        configurable: true
    });
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
                _this.eventDispatcher.dispatchEvent(new Event('Received', { data: data }));
            });
            _this.serialport.on('close', function () {
                CarSerialPort.serialStates[_this.portName] = undefined;
                console.log(_this.portName + ' closed.');
                _this.eventDispatcher.dispatchEvent('Closed');
            });
            _this.eventDispatcher.dispatchEvent('Opened');
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
    CarSerialPort.prototype.addOpenedHandler = function (handler) {
        this.eventDispatcher.addEventListener('Opened', handler);
    };
    CarSerialPort.prototype.addClosedHandler = function (handler) {
        this.eventDispatcher.addEventListener('Closed', handler);
    };
    CarSerialPort.prototype.addReceivedHandler = function (handler) {
        this.eventDispatcher.addEventListener('Received', handler);
    };
    CarSerialPort.prototype.removeOpenedHandler = function (handler) {
        this.eventDispatcher.removeEventListener('Opened', handler);
    };
    CarSerialPort.prototype.removeClosedHandler = function (handler) {
        this.eventDispatcher.removeEventListener('Closed', handler);
    };
    CarSerialPort.prototype.removeReceivedHandler = function (handler) {
        this.eventDispatcher.removeEventListener('Received', handler);
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
                return {
                    comName: port.comName,
                    pnpId: port.pnpId,
                    manufacturer: port.manufacturer,
                    state: CarSerialPort.serialStates[port.comName]
                };
            }));
        });
    };
    CarSerialPort.serialStates = {};
    return CarSerialPort;
})();
exports.CarSerialPort = CarSerialPort;
var CarTransmitter = (function (_super) {
    __extends(CarTransmitter, _super);
    function CarTransmitter(portName, bitRate) {
        var _this = this;
        _super.call(this, portName, bitRate);
        this.byteBuffer = [];
        this.addReceivedHandler(function (e) {
            var data = e.value.data;
            _.forEach(data, function (byte) { return _this.byteReceived(byte); });
        });
    }
    CarTransmitter.prototype.addDataReceivedHandler = function (handler) {
        this.eventDispatcher.addEventListener('DataReceived', handler);
    };
    CarTransmitter.prototype.removeDataReceivedHandler = function (handler) {
        this.eventDispatcher.removeEventListener('DataReceived', handler);
    };
    CarTransmitter.prototype.addMessageReceivedHandler = function (handler) {
        this.eventDispatcher.addEventListener('MessageReceived', handler);
    };
    CarTransmitter.prototype.removeMessageReceivedHandler = function (handler) {
        this.eventDispatcher.removeEventListener('MessageReceived', handler);
    };
    CarTransmitter.prototype.messageReceived = function (msg) {
        var received = CarTransmitter.parseCommandLine(msg);
        if (received != undefined && received.cmd != undefined) {
            if (received.cmd == 'put') {
                this.eventDispatcher.dispatchEvent(new Event('DataReceived', received.data));
            }
            else if (received.cmd == 'msg') {
                this.eventDispatcher.dispatchEvent(new Event('MessageReceived', received.message));
            }
        }
    };
    CarTransmitter.prototype.byteReceived = function (byte) {
        switch (byte) {
            case 0x03:
                this.messageReceived(_.reduce(this.byteBuffer, function (result, ch) { return result + String.fromCharCode(ch); }, ""));
                break;
            case 0x02:
                this.byteBuffer = [];
                break;
            default:
                this.byteBuffer.push(byte);
                break;
        }
    };
    CarTransmitter.toArrayBuffer = function (buffer) {
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
                        obj = { value: CarTransmitter.toArrayBuffer(buf) };
                        break;
                }
                var check_sum = adler32.buf(buf);
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
//# sourceMappingURL=car_comm.js.map