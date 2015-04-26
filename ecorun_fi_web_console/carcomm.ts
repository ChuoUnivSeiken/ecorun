/// <reference path="typings/tsd.d.ts" />

import _ = require('lodash');
var adler32 = require('adler-32');
var serialport = require('serialport');

export class EventDispatcher {
    listeners: any = {};
    dispatchEvent(event: any): void {
        var e: Event;
        var type: string;
        if (event instanceof Event) {
            type = event.type;
            e = event;
        } else {
            type = event;
            e = new Event(type);
        }

        if (this.listeners[type] != null) {
            e.currentTarget = this;
            for (var i: number = 0; i < this.listeners[type].length; i++) {
                var listener: EventListener = this.listeners[type][i];
                try {
                    listener.handler(e);
                } catch (error) {
                    console.error(error.stack);
                }
            }
        }
    }

    addEventListener(type: string, callback: Function, priolity: number = 0): void {
        if (this.listeners[type] == null) {
            this.listeners[type] = [];
        }

        this.listeners[type].push(new EventListener(type, callback, priolity));
        this.listeners[type].sort(function(listener1: EventListener, listener2: EventListener) {
            return listener2.priolity - listener1.priolity;
        });
    }

    removeEventListener(type: string, callback: Function): void {
        if (this.hasEventListener(type, callback)) {
            for (var i: number = 0; i < this.listeners[type].length; i++) {
                var listener: EventListener = this.listeners[type][i];
                if (listener.equalCurrentListener(type, callback)) {
                    listener.handler = null;
                    this.listeners[type].splice(i, 1);
                    return;
                }
            }
        }
    }

    clearEventListener(): void {
        this.listeners = {};
    }

    containEventListener(type: string): boolean {
        if (this.listeners[type] == null) return false;
        return this.listeners[type].length > 0;
    }

    hasEventListener(type: string, callback: Function): boolean {
        if (this.listeners[type] == null) return false;
        for (var i: number = 0; i < this.listeners[type].length; i++) {
            var listener: EventListener = this.listeners[type][i];
            if (listener.equalCurrentListener(type, callback)) {
                return true;
            }
        }
        return false;
    }
}

class EventListener {
    constructor(public type: string = null, public handler: Function = null, public priolity: number = 0) {
    }
    equalCurrentListener(type: string, handler: Function): boolean {
        if (this.type == type && this.handler == handler) {
            return true;
        }
        return false;
    }
}

export class Event {
    currentTarget: any;
    static COMPLETE: string = "complete";
    static CHANGE_PROPERTY: string = "changeProperty";
    constructor(public type: string = null, public value: any = null) {

    }
}

var serial = require('serialport');
export class CarSerialPort {
    private eventDispatcher = new EventDispatcher();
    public serialport: any = null;

    constructor(public portName: string, public bitRate: number) {
    }

    get isOpening(): boolean {
        return this.serialport != null;
    }

    public open() {
        var _serialport = new serial.SerialPort(this.portName, {
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
                this.eventDispatcher.dispatchEvent(new Event('DataReceived', { data: data }));
            });
            this.serialport.on('close', () => {
                CarSerialPort.serialStates[this.portName] = undefined;
                console.log(this.portName + ' closed.');
                this.eventDispatcher.dispatchEvent('Closed');
            });
            this.eventDispatcher.dispatchEvent('Opened');
        });
    }
    public close() {
        if (this.isOpening) {
            var _serialport = this.serialport;
            this.serialport = null;
            _serialport.close(function(error) {
                if (error !== undefined && error !== null) {
                    console.log('Serial port closing error : ' + error);
                }
            });
        }
    }
    public addOpenedHandler(handler: Function) {
        this.eventDispatcher.addEventListener('Opened', handler);
    }
    public addClosedHandler(handler: Function) {
        this.eventDispatcher.addEventListener('Closed', handler);
    }
    public addDataReceivedHandler(handler: Function) {
        this.eventDispatcher.addEventListener('DataReceived', handler);
    }
    public removeOpenedHandler(handler: Function) {
        this.eventDispatcher.removeEventListener('Opened', handler);
    }
    public removeClosedHandler(handler: Function) {
        this.eventDispatcher.removeEventListener('Closed', handler);
    }
    public removeDataReceivedHandler(handler: Function) {
        this.eventDispatcher.removeEventListener('DataReceived', handler);
    }

    public write(str: String) {
        if (this.serialport == null) {
            throw new Error("This port is not opening.");
        }
        var errfunc = function(error, results) {
            if (error !== undefined && error !== null) {
                console.log('Serial port writing error : ' + error);
            }
        };

        var stx = String.fromCharCode(0x02);
        var etx = String.fromCharCode(0x03);
        this.serialport.write(stx + str + etx, errfunc);
    }


    private static serialStates = {};

    public static getPortInfo(callback: Function) {
        serial.list((err, ports) => {
            callback(_.map(ports,(port: { comName: string; pnpId: string; manufacturer: string; }) => {
                return {
                    comName: port.comName,
                    pnpId: port.pnpId,
                    manufacturer: port.manufacturer,
                    state: CarSerialPort.serialStates[port.comName]
                };
            }));
        });
    }
}

export class CarTransmitter {
    private eventDispatcher = new EventDispatcher();
    public addDataReceivedHandler(handler: Function) {
        this.eventDispatcher.addEventListener('DataReceived', handler);
    }
    public removeDataReceivedHandler(handler: Function) {
        this.eventDispatcher.removeEventListener('DataReceived', handler);
    }
    public addMessageReceivedHandler(handler: Function) {
        this.eventDispatcher.addEventListener('MessageReceived', handler);
    }
    public removeMessageReceivedHandler(handler: Function) {
        this.eventDispatcher.removeEventListener('MessageReceived', handler);
    }

    private messageReceived(msg) {
        var received: any = CarTransmitter.parseCommandLine(msg);
        if (received != undefined && received.cmd != undefined) {
            if (received.cmd == 'put') {
                this.eventDispatcher.dispatchEvent(new Event('DataReceived', received.data));
            } else if (received.cmd == 'msg') {
                this.eventDispatcher.dispatchEvent(new Event('MessageReceived', received.message));
            }
        }
    }

    private byteBuffer = [];

    private byteReceived(byte: number) {
        switch (byte) {
            case 0x03:
                this.messageReceived(_.reduce(this.byteBuffer, (result, ch) => result + String.fromCharCode(ch), ""));
                break;
            case 0x02:
                this.byteBuffer = [];
                break;
            default:
                this.byteBuffer.push(byte);
                break;
        }
    }

    constructor(private carSerialport: CarSerialPort) {
        carSerialport.addDataReceivedHandler((e) => {
            var data: Array<number> = e.value.data;
            _.forEach(data, (byte: number) => this.byteReceived(byte));
        });
    }

    private static toArrayBuffer(buffer: Buffer): ArrayBuffer {
        var ab = new ArrayBuffer(buffer.length);
        var view = new Uint8Array(ab);
        for (var i = 0; i < buffer.length; ++i) {
            view[i] = buffer[i];
        }
        return ab;
    }

    public requestData(id: String) {
        if (serialport != null) {
            this.carSerialport.write('get ' + id);
        }
    }

    private static parseCommandLine(line: string): any {
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
                        obj = { value: CarTransmitter.toArrayBuffer(buf) }
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
        private values: number[]= [],
        private names: string[]= []) {
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
