/// <reference path="typings/tsd.d.ts" />
var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};
var _ = require('lodash');
var adler32 = require('adler-32');
var CarTransmitter = (function () {
    function CarTransmitter() {
    }
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
                var stream = new StructReader(buf);
                var obj = null;
                switch (id) {
                    case 'engine_data':
                        obj = stream
                            .readUInt32('rev')
                            .readUInt32('is_fuel_cut')
                            .readUInt32('is_af_rich')
                            .readUInt32('th')
                            .readUInt32('oil_temp')
                            .readUInt32('current_total_injected_time')
                            .readUInt32('current_inject_started_count')
                            .readUInt32('current_inject_ended_count')
                            .toObject();
                        break;
                    case 'car_data':
                        obj = stream
                            .readUInt32('vattery_voltage')
                            .readUInt32('wheel_count')
                            .readUInt32('wheel_rotation_period')
                            .toObject();
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
})();
exports.CarTransmitter = CarTransmitter;
var StructReader = (function () {
    function StructReader(buffer, offset, values, names) {
        if (offset === void 0) { offset = 0; }
        if (values === void 0) { values = []; }
        if (names === void 0) { names = []; }
        this.buffer = buffer;
        this.offset = offset;
        this.values = values;
        this.names = names;
    }
    StructReader.prototype.readInt8 = function (name) {
        if (this.offset + 1 > this.buffer.length) {
            return new StructReaderNone();
        }
        return new StructReader(this.buffer, this.offset + 1, this.values.concat(this.buffer.readInt8(this.offset)), this.names.concat(name));
    };
    StructReader.prototype.readUInt32 = function (name) {
        if (this.offset + 4 > this.buffer.length) {
            return new StructReaderNone();
        }
        return new StructReader(this.buffer, this.offset + 4, this.values.concat(this.buffer.readUInt32LE(this.offset)), this.names.concat(name));
    };
    StructReader.prototype.readInt16 = function (name) {
        if (this.offset + 2 > this.buffer.length) {
            return new StructReaderNone();
        }
        return new StructReader(this.buffer, this.offset + 2, this.values.concat(this.buffer.readInt16LE(this.offset)), this.names.concat(name));
    };
    StructReader.prototype.toObject = function () {
        return _.object(this.names, this.values);
    };
    return StructReader;
})();
exports.StructReader = StructReader;
var StructReaderNone = (function (_super) {
    __extends(StructReaderNone, _super);
    function StructReaderNone() {
        _super.call(this, null);
    }
    StructReaderNone.prototype.readInt8 = function (name) {
        return new StructReaderNone();
    };
    StructReaderNone.prototype.readUInt32 = function (name) {
        return new StructReaderNone();
    };
    StructReaderNone.prototype.readInt16 = function (name) {
        return new StructReaderNone();
    };
    return StructReaderNone;
})(StructReader);
exports.StructReaderNone = StructReaderNone;
