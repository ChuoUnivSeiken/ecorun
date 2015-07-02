/// <reference path="typings/tsd.d.ts" />
var __extends = (this && this.__extends) || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
};
var d3 = require("d3");
var rect = (function () {
    function rect(top, right, bottom, left) {
        if (top === void 0) { top = 0; }
        if (right === void 0) { right = 0; }
        if (bottom === void 0) { bottom = 0; }
        if (left === void 0) { left = 0; }
        this.top = top;
        this.right = right;
        this.bottom = bottom;
        this.left = left;
    }
    return rect;
})();
exports.rect = rect;
var vec2d = (function () {
    function vec2d(x, y) {
        if (x === void 0) { x = 0; }
        if (y === void 0) { y = 0; }
        this.x = x;
        this.y = y;
    }
    return vec2d;
})();
exports.vec2d = vec2d;
var margins = (function () {
    function margins(top, right, bottom, left) {
        if (top === void 0) { top = 0; }
        if (right === void 0) { right = 0; }
        if (bottom === void 0) { bottom = 0; }
        if (left === void 0) { left = 0; }
        this.top = top;
        this.right = right;
        this.bottom = bottom;
        this.left = left;
    }
    return margins;
})();
exports.margins = margins;
var size2d = (function () {
    function size2d(width, height) {
        if (width === void 0) { width = 0; }
        if (height === void 0) { height = 0; }
        this.width = 0;
        this.height = 0;
        this.width = width;
        this.height = height;
    }
    return size2d;
})();
exports.size2d = size2d;
var ChartElement = (function () {
    function ChartElement(el, options) {
        this.margins = new margins(25, 50, 25, 50);
        this.el = d3.select(el);
        if (options) {
            if (options["margins"] && (options["margins"] instanceof margins)) {
                this.margins = options["margins"];
            }
        }
        if (el != null) {
            this.domel = el;
            this.el = d3.select(this.domel);
        }
        else {
            this.domel = document.createElement('div');
            this.el = d3.select(this.domel)
                .attr('width', 300)
                .attr('height', 300);
        }
    }
    Object.defineProperty(ChartElement.prototype, "size", {
        get: function () {
            return new size2d(this.domel.clientWidth, this.domel.clientHeight);
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(ChartElement.prototype, "clientSize", {
        get: function () {
            return new size2d(this.size.width - (this.margins.left + this.margins.right), this.size.height - (this.margins.top + this.margins.bottom));
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(ChartElement.prototype, "clientRect", {
        get: function () {
            return new rect(this.margins.top, this.size.width - this.margins.right, this.size.height - this.margins.bottom, this.margins.left);
        },
        enumerable: true,
        configurable: true
    });
    return ChartElement;
})();
exports.ChartElement = ChartElement;
var ChartSVG = (function (_super) {
    __extends(ChartSVG, _super);
    function ChartSVG(el, options) {
        _super.call(this, el, options);
        this.svg = this.el
            .append("svg")
            .attr("width", this.size.width)
            .attr("height", this.size.height);
    }
    return ChartSVG;
})(ChartElement);
exports.ChartSVG = ChartSVG;
var ChartCanvas = (function (_super) {
    __extends(ChartCanvas, _super);
    function ChartCanvas(el, options) {
        var _this = this;
        _super.call(this, el, options);
        this.canvas = d3.select(document.createElement("canvas"));
        this.canvas.style({
            top: this.margins.top + "px",
            left: this.margins.left + "px",
        }).attr("width", this.clientSize.width)
            .attr("height", this.clientSize.height);
        this.el.node().appendChild(this.canvas.node());
        var c = this.canvas.node();
        this.context = c.getContext("2d");
        var queue = null, wait = 300;
        window.addEventListener('resize', function () {
            clearTimeout(queue);
            queue = setTimeout(function () {
                _this.canvas
                    .attr("width", _this.clientSize.width)
                    .attr("height", _this.clientSize.height);
            }, wait);
        }, false);
    }
    return ChartCanvas;
})(ChartElement);
exports.ChartCanvas = ChartCanvas;
var Chart = (function (_super) {
    __extends(Chart, _super);
    function Chart(el, options) {
        var _this = this;
        _super.call(this, el, options);
        this.values = [];
        this._querySize = 250;
        this.svg = this.el.insert('svg', ':first-child')
            .attr('width', this.size.width)
            .attr('height', this.size.height)
            .style('z-index', '1000');
        if (this.el.style('position') != 'absolute' && this.el.style('position') != 'relative') {
            this.el.style('position', 'relative');
        }
        this.canvas.style({ position: 'absolute', 'z-index': '999' });
        var now = new Date();
        this.buildAxes();
        var updateTimer = setInterval(function () {
            _this.svgXAxis.transition().duration(500).ease("linear").call(_this.bottomAxis);
            _this.svgYAxis.transition().duration(500).ease("linear").call(_this.leftAxis);
        }, 500);
        var drawTimer = setInterval(function () { return _this.draw(); }, 33);
    }
    Object.defineProperty(Chart.prototype, "queueSize", {
        get: function () {
            return this._querySize;
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(Chart.prototype, "xScale", {
        get: function () {
            var now = new Date();
            return d3.scale.linear()
                .domain([now.getTime() - 1000 * 25, now.getTime() - 1000])
                .range([0, this.clientSize.width]);
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(Chart.prototype, "yScale", {
        get: function () {
            var max = 0;
            if (this.values.length > 0) {
                if (this.values.length == 1) {
                    max = d3.max([this.values[0].y, 0]);
                }
                else {
                    max = d3.max(this.values, function (d) { return d.y; });
                }
            }
            max = 100;
            return d3.scale.linear()
                .domain([0, max])
                .range([this.clientSize.height, 0]);
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(Chart.prototype, "bottomAxis", {
        get: function () {
            return d3.svg.axis()
                .scale(this.xScale)
                .orient("bottom")
                .tickFormat(function (value) {
                var date = new Date(value);
                return date.toLocaleTimeString();
            })
                .ticks(5);
        },
        enumerable: true,
        configurable: true
    });
    Object.defineProperty(Chart.prototype, "leftAxis", {
        get: function () {
            return d3.svg.axis()
                .scale(this.yScale)
                .orient("left")
                .ticks(5);
        },
        enumerable: true,
        configurable: true
    });
    Chart.prototype.buildAxes = function () {
        this.svg.selectAll(".axis").remove();
        this.svgXAxis = this.svg.append("g")
            .attr("transform", "translate(" + this.clientRect.left + "," + this.clientRect.bottom + ")")
            .attr("class", "axis")
            .call(this.bottomAxis);
        this.svgYAxis = this.svg.append("g")
            .attr("transform", "translate(" + this.clientRect.left + "," + this.clientRect.top + ")")
            .attr("class", "axis")
            .call(this.leftAxis);
    };
    Chart.prototype.push = function (y) {
        var now = new Date();
        this.values.push(new vec2d(now.getTime(), y));
        if (this.values.length > this.queueSize) {
            this.values.shift();
        }
    };
    Chart.prototype.draw = function () {
        if (this.values.length > 0) {
            var now = new Date();
            this.xScale = d3.scale.linear()
                .domain([now.getTime() - 1000 * 25, now.getTime() - 1000])
                .range([0, this.clientSize.width]);
            this.context.clearRect(0, 0, this.size.width, this.size.height);
            this.context.beginPath();
            this.context.strokeStyle = '#1f77b4';
            this.context.lineWidth = 3;
            this.context.moveTo(this.xScale(this.values[0].x), this.yScale(this.values[0].y));
            var prevPt = this.values[0];
            for (var i = 1; i < this.values.length; i++) {
                var currentPt = this.values[i];
                if (currentPt.x != prevPt.x || currentPt.y != prevPt.y) {
                    this.context.lineTo(this.xScale(this.values[i].x), this.yScale(this.values[i].y));
                }
            }
            this.context.stroke();
        }
    };
    return Chart;
})(ChartCanvas);
exports.Chart = Chart;
