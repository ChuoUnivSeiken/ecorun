/// <reference path="typings/tsd.d.ts" />
var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};
var _ = require('lodash');
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
    StructReader.prototype.structs = function () {
        return _.object(this.names, this.values);
    };
    ;
    return StructReader;
})();
exports.StructReader = StructReader;
var StructReaderNone = (function (_super) {
    __extends(StructReaderNone, _super);
    function StructReaderNone() {
        _super.call(this, null);
    }
    return StructReaderNone;
})(StructReader);
exports.StructReaderNone = StructReaderNone;
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
