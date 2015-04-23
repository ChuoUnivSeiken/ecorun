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
				y: 0
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

$(function() {
	var speed_data = new RealTimeData(1);
	var speed_chart = $('#chart_speed').epoch({
		type: 'time.line',
		data: speed_data.history(),
		axes: ['left', 'bottom', 'right']
	});

    var voltage_data = new RealTimeData(1);
    var voltage_chart = $('#chart_vattery_voltage').epoch({
        type: 'time.line',
        data: voltage_data.history(),
        axes: ['left', 'bottom', 'right']
    });
    
    var rev_data = new RealTimeData(1);
    var rev_chart = $('#chart_rev').epoch({
        type: 'time.line',
        data: rev_data.history(),
        axes: ['left', 'bottom', 'right']
    });
    
    var throttle_data = new RealTimeData(1);
    var throttle_chart = $('#chart_throttle').epoch({
        type: 'time.line',
        data: throttle_data.history(),
        axes: ['left', 'bottom', 'right']
    });
    
    var oil_temp_data = new RealTimeData(1);
    var oil_temp_chart = $('#chart_oil_temp').epoch({
        type: 'time.line',
        data: oil_temp_data.history(),
        axes: ['left', 'bottom', 'right']
    });

    var temp_data = new RealTimeData(1);
    var temp_chart = $('#chart_temp').epoch({
        type: 'time.line',
        data: temp_data.history(),
        axes: ['left', 'bottom', 'right']
    });
    
    var data_apply = function(name, value, timestamp) {
        switch (name) {
            case 'rev':
                $('#td_rev').html(value);
                var entry = [];
                entry.push({time: ((new Date()).getTime() / 1000), y: value});
                rev_chart.push(entry);
                break;
            case 'th':
                $('#td_th').html(value);
                var entry = [];
                entry.push({time: ((new Date()).getTime() / 1000), y: value});
                throttle_chart.push(entry);
                break;
            case 'oil_temp':
                var B = 3400;
                var lnR0 = Math.log(1.981);
                var denom = Math.log(value / 1000.0) - lnR0 + (B / 301.0);
                var oil_temp = B / denom - 273.0;
                $('#td_oil_temp').html(oil_temp);
                var entry = [];
                entry.push({time: ((new Date()).getTime() / 1000), y: oil_temp});
                oil_temp_chart.push(entry);

                break;
            case 'vattery_voltage':
                var vattery_voltage = value / 1000.0;
                $('#td_vattery_voltage').html(vattery_voltage);
                var entry = [];
                entry.push({time: ((new Date()).getTime() / 1000), y: vattery_voltage});
                voltage_chart.push(entry);
                break;
        }
    };

    var data_received = function(data) {
        if (data === undefined || data === null) {
            return;
        }
        console.log(data);
        switch (data.id) {
            case 'car_data':
            case 'engine_data':
                _.forEach(data.value, function(value, prop) {
                    data_apply(prop, value, data.timestamp.microsec + 1000000 * data.timestamp.sec);
                });
                break;
        }
    };

    window.data_received = data_received;
});

