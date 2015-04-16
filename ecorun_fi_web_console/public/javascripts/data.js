/*
 * Class for generating real-time data for the area, line, and bar plots.
 */
var RealTimeData = function(layers) {
	this.layers = layers;
	this.timestamp = ((new Date()).getTime() / 1000) | 0;
};

RealTimeData.prototype.rand = function() {
	return parseInt(Math.random() * 100) + 50;
};

RealTimeData.prototype.history = function(entries) {
	if (typeof(entries) != 'number' || !entries) {
		entries = 60;
	}

	var history = [];
	for (var k = 0; k < this.layers; k++) {
		history.push({
			values: []
		});
	}

	for (var i = 0; i < entries; i++) {
		for (var j = 0; j < this.layers; j++) {
			history[j].values.push({
				time: this.timestamp,
				y: this.rand()
			});
		}
		this.timestamp++;
	}

	return history;
};

RealTimeData.prototype.next = function() {
	var entry = [];
	for (var i = 0; i < this.layers; i++) {
		entry.push({
			time: this.timestamp,
			y: this.rand()
		});
	}
	this.timestamp++;
	return entry;
};

window.RealTimeData = RealTimeData;

/*
 * Gauge Data Generator.
 */
var GaugeData = function() {};

GaugeData.prototype.next = function() {
	return Math.random();
};

window.GaugeData = GaugeData;

/*
 * Heatmap Data Generator.
 */

var HeatmapData = function(layers) {
	this.layers = layers;
	this.timestamp = ((new Date()).getTime() / 1000) | 0;
};

window.normal = function() {
	var U = Math.random(),
		V = Math.random();
	return Math.sqrt(-2 * Math.log(U)) * Math.cos(2 * Math.PI * V);
};

HeatmapData.prototype.rand = function() {
	var histogram = {};

	for (var i = 0; i < 1000; i++) {
		var r = parseInt(normal() * 12.5 + 50);
		if (!histogram[r]) {
			histogram[r] = 1;
		} else {
			histogram[r] ++;
		}
	}

	return histogram;
};

HeatmapData.prototype.history = function(entries) {
	if (typeof(entries) != 'number' || !entries) {
		entries = 60;
	}

	var history = [];
	for (var k = 0; k < this.layers; k++) {
		history.push({
			values: []
		});
	}

	for (var i = 0; i < entries; i++) {
		for (var j = 0; j < this.layers; j++) {
			history[j].values.push({
				time: this.timestamp,
				histogram: this.rand()
			});
		}
		this.timestamp++;
	}

	return history;
};

HeatmapData.prototype.next = function() {
	var entry = [];
	for (var i = 0; i < this.layers; i++) {
		entry.push({
			time: this.timestamp,
			histogram: this.rand()
		});
	}
	this.timestamp++;
	return entry;
};

window.HeatmapData = HeatmapData;

$(function() {
	var speed_data = new RealTimeData(1);
	var speed_chart = $('#speed_chart').epoch({
		type: 'time.line',
		data: speed_data.history(),
		axes: ['left', 'bottom', 'right']
	});

	setInterval(function() {
		speed_chart.push(speed_data.next());
	}, 1000);
	speed_chart.push(speed_data.next());

	var voltage_data = new RealTimeData(1);
	var voltage_chart = $('#voltage_chart').epoch({
		type: 'time.line',
		data: voltage_data.history(),
		axes: ['left', 'bottom', 'right']
	});

	setInterval(function() {
		voltage_chart.push(voltage_data.next());
	}, 1000);
	voltage_chart.push(voltage_data.next());

	var rev_data = new RealTimeData(1);
	var rev_chart = $('#rev_chart').epoch({
		type: 'time.line',
		data: rev_data.history(),
		axes: ['left', 'bottom', 'right']
	});

	setInterval(function() {
		rev_chart.push(rev_data.next());
	}, 1000);
	rev_chart.push(rev_data.next());
});

var data_apply = function(name, value) {
	switch (name) {
		case 'rev':
			$('#td_rev').html(value);
			break;
		case 'th':
			$('#td_th').html(value);
			break;
		case 'oil_temp':
			var B = 3400;
			var lnR0 = Math.log(1.981);
			var denom = Math.log(value / 1000.0) - lnR0 + (B / 301.0);
			var oil_temp = B / denom - 273.0;
			$('#td_oil_temp').html(oil_temp);
			break;
		case 'vattery_voltage':
			var vattery_voltage = value / 1000.0;
			$('#td_vattery_voltage').html(vattery_voltage);
			break;
	}
};

var data_received = function(data) {
	if (data === undefined || data === null) {
		return;
	}
	console.log(data);
	switch (data.name) {
		case 'rrcardata':
		case 'rrenginedata':
			_.forEach(data.value, function(value, prop) {
				data_apply(prop, value);
			});
			break;
	}
};