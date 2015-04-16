/// <reference path="typings/tsd.d.ts" />

import _ = require('lodash')
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
    

    structs():any {
	    return _.object(this.names, this.values);
    };

}

export class StructReaderNone extends StructReader {
    constructor() {
        super(null)
    }
}
/*
var _ = require('lodash')

function BufferStructReaderIter(buf, off, vals, nams) {
	this.buffer = buf;
	this.offset = off;
	this.values = vals;
	this.names = nams;
}

BufferStructReaderIter.prototype.readInt8 = function(name) {
	if (this.offset + 1 > this.buffer.length) {
		return new BufferStructReaderNone();
	}
	return new BufferStructReaderIter(this.buffer, this.offset + 1, this.values
		.concat(this.buffer.readInt8(this.offset)), this.names.concat(name));
};

BufferStructReaderIter.prototype.readInt16 = function(name) {
	if (this.offset + 2 > this.buffer.length) {
		return new BufferStructReaderNone();
	}
	return new BufferStructReaderIter(this.buffer, this.offset + 2, this.values
		.concat(this.buffer.readInt16LE(this.offset)), this.names
		.concat(name));
};

BufferStructReaderIter.prototype.readInt32 = function(name) {
	if (this.offset + 4 > this.buffer.length) {
		return new BufferStructReaderNone();
	}
	return new BufferStructReaderIter(this.buffer, this.offset + 4, this.values
		.concat(this.buffer.readInt32LE(this.offset)), this.names
		.concat(name));
};

BufferStructReaderIter.prototype.readUInt16 = function(name) {
	if (this.offset + 2 > this.buffer.length) {
		return new BufferStructReaderNone();
	}
	return new BufferStructReaderIter(this.buffer, this.offset + 2, this.values
		.concat(this.buffer.readUInt16LE(this.offset)), this.names
		.concat(name));
};

BufferStructReaderIter.prototype.readUInt32 = function(name) {
	if (this.offset + 4 > this.buffer.length) {
		return new BufferStructReaderNone();
	}
	return new BufferStructReaderIter(this.buffer, this.offset + 4, this.values
		.concat(this.buffer.readUInt32LE(this.offset)), this.names
		.concat(name));
};

BufferStructReaderIter.prototype.structs = function() {
	return _.object(this.names, this.values);
};

function BufferStructReaderNone() {
	this.buffer = null;
	this.offset = 0;
	this.values = [];
	this.names = [];
}

BufferStructReaderNone.prototype = Object.create(BufferStructReaderIter.prototype);
BufferStructReaderNone.prototype.constructor = BufferStructReaderNone;

BufferStructReaderNone.prototype.readInt8 = function(name) {
	return new BufferStructReaderNone();
};

BufferStructReaderNone.prototype.readInt16 = function(name) {
	return new BufferStructReaderNone();
};

BufferStructReaderNone.prototype.readInt32 = function(name) {
	return new BufferStructReaderNone();
};

BufferStructReaderNone.prototype.readUInt16 = function(name) {
	return new BufferStructReaderNone();
};

BufferStructReaderNone.prototype.readUInt32 = function(name) {
	return new BufferStructReaderNone();
};

BufferStructReaderNone.prototype.structs = function() {
	return null;
};

module.exports = {
	BufferStructReader: BufferStructReader
};

*/
