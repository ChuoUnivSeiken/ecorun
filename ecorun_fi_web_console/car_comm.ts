/// <reference path="typings/tsd.d.ts" />

import _ = require('lodash');
var serialport = require('serialport');
var adler32 = require('adler-32');
import events = require('events');

export class CarSerialPort extends events.EventEmitter {
    private serialport: any = null;

    constructor(public portName: string, public bitRate: number) {
        super();
    }

    get isOpening(): boolean {
        return this.serialport != null;
    }

    private byteBuffer = [];

    private byteReceived(byte: number) {
        switch (byte) {
            case 0x03:
                this.dataReceived(_.reduce(this.byteBuffer, (result, ch) => result + String.fromCharCode(ch), ""));
                break;
            case 0x02:
                this.byteBuffer = [];
                break;
            default:
                this.byteBuffer.push(byte);
                break;
        }
    }

    protected dataReceived(data: string) {
        this.emit('data', data);
    }

    open() {
        var _serialport = new serialport.SerialPort(this.portName, {
            baudrate: this.bitRate,
            dataBits: 8,
            parity: 'none',
            stopBits: 1,
            flowControl: false
        }, (error) => {
                if (error !== undefined && error !== null) {
                    console.log('serial port opening error : ' + error);
                }
            });
        _serialport.on('open', () => {
            CarSerialPort.serialStates[this.portName] = 'connect';
            console.log(this.portName + ' opened.');
            this.serialport = _serialport;
            this.serialport.on('data', (data) => {
                _.forEach(data, (byte: number) => this.byteReceived(byte));
            });

            this.serialport.on('close', () => {
                CarSerialPort.serialStates[this.portName] = undefined;
                console.log(this.portName + ' closed.');
                this.emit('closed')
            });
            this.emit('opened')
        });
    }

    close() {
        if (this.isOpening) {
            var _serialport = this.serialport;
            this.serialport = null;
            _serialport.close((error) => {
                if (error !== undefined && error !== null) {
                    console.log('Serial port closing error : ' + error);
                }
            });
        }
    }

    write(str: String) {
        if (!this.isOpening) {
            throw new Error("This port is not opening.");
        }
        var errfunc = (error, results) => {
            if (error !== undefined && error !== null) {
                console.log('Serial port writing error : ' + error);
            }
        };

        var stx = String.fromCharCode(0x02);
        var etx = String.fromCharCode(0x03);
        this.serialport.write(stx + str + etx, errfunc);
    }

    private static serialStates: { [key: string]: string; } = {};

    static getPortInfo(callback: Function) {
        serialport.list((err, ports) => {
            callback(_.map(ports, (port: { comName: string; pnpId: string; manufacturer: string; }) => {
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
    }
}

export class CarTransmitter extends CarSerialPort {

    constructor(portName: string, bitRate: number) {
        super(portName, bitRate);
    }

    protected dataReceived(data: string) {
        super.dataReceived(data);

        var received: any = CarTransmitter.parseCommandLine(data);
        if (received != undefined && received.cmd != undefined) {
            if (received.cmd == 'put') {
                this.emit('obj', received.data);
            } else if (received.cmd == 'msg') {
                this.emit('msg', received.message);
            }
        }
    }

    private static toByteBuffer(buffer: Buffer) {
        var ab = new ArrayBuffer(buffer.length);
        var view = new Uint8Array(ab);
        for (var i = 0; i < buffer.length; ++i) {
            view[i] = buffer[i];
        }
        return ab;
    }

    requestData(id: string) {
        if (this.isOpening) {
            this.write('get ' + id);
        }
    }

    sendData(id: string, data: string) {
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
    }

    private static parseCommandLine(line: string): Object {
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
                }
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
                        obj = stream
                            .readUInt32('rev')
                            .readUInt32('is_fuel_cut')
                            .readUInt32('is_af_rich')
                            .readUInt32('th')
                            .readUInt32('oil_temp')
                            .readUInt32('intake_temp')
                            .readUInt32('vacuum')
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
                    }
                }
                break;
            default:
                console.log('Invalid command : ', command);
                return null;
                break;
        }

    }
}

export class ObjectReader {
    constructor(
        private buffer: Buffer,
        private offset: number = 0,
        private values: number[] = [],
        private names: string[] = []) {
    }

    public readInt8(name: string): ObjectReader {
        if (this.offset + 1 > this.buffer.length) {
            return new ObjectReaderNone();
        }

        return new ObjectReader(
            this.buffer,
            this.offset + 1,
            this.values.concat(this.buffer.readInt8(this.offset)),
            this.names.concat(name)
            );
    }

    public readUInt32(name: string): ObjectReader {
        if (this.offset + 4 > this.buffer.length) {
            return new ObjectReaderNone();
        }

        return new ObjectReader(
            this.buffer,
            this.offset + 4,
            this.values.concat(this.buffer.readUInt32LE(this.offset)),
            this.names.concat(name)
            );
    }

    public readInt16(name: string): ObjectReader {
        if (this.offset + 2 > this.buffer.length) {
            return new ObjectReaderNone();
        }
        return new ObjectReader(
            this.buffer,
            this.offset + 2,
            this.values.concat(this.buffer.readInt16LE(this.offset)),
            this.names.concat(name)
            );
    }

    toObject(): Object {
        return _.object(this.names, this.values);
    }
}

export class ObjectReaderNone extends ObjectReader {
    constructor() {
        super(null)
    }

    public readInt8(name): ObjectReader {
        return new ObjectReaderNone();
    }

    public readUInt32(name): ObjectReader {
        return new ObjectReaderNone();
    }

    public readInt16(name): any {
        return new ObjectReaderNone();
    }
}
