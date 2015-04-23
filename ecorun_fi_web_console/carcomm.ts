/// <reference path="typings/tsd.d.ts" />

import _ = require('lodash')
var adler32 = require('adler-32')

export class CarTransmitter {
    public static parseCommandLine(line): any {
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
                if (tokens.length !==7) {
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
                                microsec:microsec,
                                sec:sec
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

export class StructReader {
    constructor(
            private buffer:Buffer,
            private offset:number = 0,
            private values:number[] = [],
            private names:string[] = []) {
    }
    
    readInt8(name): StructReader {
        if (this.offset + 1 > this.buffer.length) {
            return new StructReaderNone();
        }

        return new StructReader(
                this.buffer,
                this.offset + 1,
                this.values.concat(this.buffer.readInt8(this.offset)),
                this.names.concat(name)
                );
    }

    readUInt32(name): StructReader {
        if (this.offset + 4 > this.buffer.length) {
            return new StructReaderNone();
        }

        return new StructReader(
                this.buffer,
                this.offset + 4,
                this.values.concat(this.buffer.readUInt32LE(this.offset)),
                this.names.concat(name)
                );
    }

    readInt16(name):any {
	    if (this.offset + 2 > this.buffer.length) {
		    return new StructReaderNone();
	    }
	    return new StructReader(
                this.buffer, 
                this.offset + 2,
                this.values.concat(this.buffer.readInt16LE(this.offset)),
                this.names.concat(name)
                );
    }

    toObject():any {
	    return _.object(this.names, this.values);
    }
}

export class StructReaderNone extends StructReader {
    constructor() {
        super(null)
    }

    readInt8(name): StructReader {
        return new StructReaderNone();
    }

    readUInt32(name): StructReader {
        return new StructReaderNone();
    }

    readInt16(name):any {
        return new StructReaderNone();
    }
}
