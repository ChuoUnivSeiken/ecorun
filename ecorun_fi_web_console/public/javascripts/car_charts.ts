/// <reference path="typings/tsd.d.ts" />

import d3 = require("d3");

export class rect {
    constructor(top: number = 0, right: number = 0, bottom: number = 0, left: number = 0) {
        this.top = top;
        this.right = right;
        this.bottom = bottom;
        this.left = left;
    }

    top: number;
    right: number;
    bottom: number;
    left: number;
}

export class vec2d {
    constructor(x: number = 0, y: number = 0) {
        this.x = x;
        this.y = y;
    }

    x: number;
    y: number;
}

export class margins {
    constructor(top: number = 0, right: number = 0, bottom: number = 0, left: number = 0) {
        this.top = top;
        this.right = right;
        this.bottom = bottom;
        this.left = left;
    }

    top: number;
    right: number;
    bottom: number;
    left: number;
}

export class size2d {
    constructor(width: number = 0, height: number = 0) {
        this.width = width;
        this.height = height;
    }

    width: number = 0;
    height: number = 0;
}

export class ChartElement {
    protected margins: margins = new margins(25, 50, 25, 50);
    protected el: D3.Selection;
    protected domel: HTMLElement;

    get size(): size2d {
        return new size2d(this.domel.clientWidth, this.domel.clientHeight);
    }

    get clientSize(): size2d {
        return new size2d(this.size.width - (this.margins.left + this.margins.right), this.size.height - (this.margins.top + this.margins.bottom));
    }

    get clientRect(): rect {
        return new rect(this.margins.top, this.size.width - this.margins.right, this.size.height - this.margins.bottom, this.margins.left);
    }

    constructor(el: HTMLElement, options?: Object) {
        this.el = d3.select(el);

        if (options) {
            if (options["margins"] && (options["margins"] instanceof margins)) {
                this.margins = options["margins"];
            }
        }

        if (el != null) {
            this.domel = el;
            this.el = d3.select(this.domel);
        } else {
            this.domel = document.createElement('div');
            this.el = d3.select(this.domel)
                .attr('width', 300)
                .attr('height', 300)
        }
    }
}

export class ChartSVG extends ChartElement {
    protected svg: D3.Selection;

    constructor(el: HTMLElement, options?: Object) {
        super(el, options);

        this.svg = this.el
            .append("svg")
            .attr("width", this.size.width)
            .attr("height", this.size.height);
    }
}

export class ChartCanvas extends ChartElement {
    protected canvas: D3.Selection;
    protected context: CanvasRenderingContext2D;

    constructor(el: HTMLElement, options?: Object) {
        super(el, options);

        this.canvas = d3.select(document.createElement("canvas"));
        this.canvas.style({
            top: this.margins.top + "px",
            left: this.margins.left + "px",
        }).attr("width", this.clientSize.width)
            .attr("height", this.clientSize.height);

        this.el.node().appendChild(this.canvas.node());
        var c = <HTMLCanvasElement>this.canvas.node();
        this.context = c.getContext("2d");

        var queue = null, // キューをストック 
            wait = 300; // 0.3秒後に実行の場合 
 
        window.addEventListener('resize', () => {
            // イベント発生の都度、キューをキャンセル 
            clearTimeout(queue);
 
            // waitで指定したミリ秒後に所定の処理を実行 
            // 経過前に再度イベントが発生した場合
            // キューをキャンセルして再カウント 
            queue = setTimeout(() => {
                this.canvas
                    .attr("width", this.clientSize.width)
                    .attr("height", this.clientSize.height);
            }, wait);
        }, false);
    }
}

export class Chart extends ChartCanvas {
    protected svg: D3.Selection;
    protected values: vec2d[] = [];
    private _querySize: number = 250;
    protected svgXAxis: D3.Selection;
    protected svgYAxis: D3.Selection;

    get queueSize(): number {
        return this._querySize;
    }
    
    protected get xScale(): D3.Scale.LinearScale {
        var now = new Date();
        return d3.scale.linear()
            .domain([now.getTime() - 1000 * 25, now.getTime() - 1000])
            .range([0, this.clientSize.width]);
    }

    protected get yScale(): D3.Scale.LinearScale {
        var max = 0;
        if (this.values.length > 0) {
            if (this.values.length == 1) {
                max = d3.max([this.values[0].y, 0]);
            } else {
                max = d3.max(this.values,(d) => d.y);
            }
        }
        max = 100;
        return d3.scale.linear()
            .domain([0, max])
            .range([this.clientSize.height, 0]);
    }

    protected get bottomAxis(): D3.Svg.Axis {
        return d3.svg.axis()
            .scale(this.xScale)
            .orient("bottom")
            .tickFormat((value) => {
                var date = new Date(value);
                return date.toLocaleTimeString();
            })
            .ticks(5);
    }

    protected get leftAxis(): D3.Svg.Axis {
        return d3.svg.axis()
            .scale(this.yScale)
            .orient("left")
            .ticks(5);
    }

    constructor(el: HTMLElement, options?: Object) {
        super(el, options);

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

        var updateTimer = setInterval(() => {
            this.svgXAxis.transition().duration(500).ease("linear").call(this.bottomAxis);
            this.svgYAxis.transition().duration(500).ease("linear").call(this.leftAxis);
        }, 500);

        var drawTimer = setInterval(() => this.draw(), 33);
    }

    private buildAxes() {
        this.svg.selectAll(".axis").remove();

        this.svgXAxis = this.svg.append("g")
            .attr("transform", "translate(" + this.clientRect.left + "," + this.clientRect.bottom + ")")
            .attr("class", "axis")
            .call(this.bottomAxis);

        this.svgYAxis = this.svg.append("g")
            .attr("transform", "translate(" + this.clientRect.left + "," + this.clientRect.top + ")")
            .attr("class", "axis")
            .call(this.leftAxis);
    }

    push(y: number) {
        var now = new Date();
        this.values.push(new vec2d(now.getTime(), y));

        if (this.values.length > this.queueSize) {
            this.values.shift();
        }
    }

    protected draw() {
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
    }
}
