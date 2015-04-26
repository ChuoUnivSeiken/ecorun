var make_initial_data = function() {
    var timestamp = ((new Date()).getTime() / 1000) | 0;
    var entries = 60;
    var values = [];
    for (var i = 0; i < entries; i++) {
        values.push({
            time: timestamp,
            y: 0
        });
        timestamp++;
    }

    var history = [{
        values: values
    }];

    return history;
}

$(function() {
    var initial_data = make_initial_data();

    var speed_chart = $('#chart_speed').epoch({
        type: 'time.line',
        data: initial_data,
        axes: ['left', 'bottom', 'right']
    });

    var voltage_chart = $('#chart_vattery_voltage').epoch({
        type: 'time.line',
        data: initial_data,
        axes: ['left', 'bottom', 'right']
    });

    var ave_speed_chart = $('#chart_ave_speed').epoch({
        type: 'time.line',
        data: initial_data,
        axes: ['left', 'bottom', 'right']
    });

    var distance_chart = $('#chart_distance').epoch({
        type: 'time.line',
        data: initial_data,
        axes: ['left', 'bottom', 'right']
    });

    var rev_chart = $('#chart_rev').epoch({
        type: 'time.line',
        data: initial_data,
        axes: ['left', 'bottom', 'right']
    });

    var throttle_chart = $('#chart_throttle').epoch({
        type: 'time.line',
        data: initial_data,
        axes: ['left', 'bottom', 'right']
    });

    var oil_temp_chart = $('#chart_oil_temp').epoch({
        type: 'time.line',
        data: initial_data,
        axes: ['left', 'bottom', 'right']
    });

    var temp_chart = $('#chart_temp').epoch({
        type: 'time.line',
        data: initial_data,
        axes: ['left', 'bottom', 'right']
    });

    var data_apply = function(name, value, timestamp) {
        switch (name) {
            case 'rev':
                $('#td_rev').html(value);
                $('#progress_rev').val(value);
                var entry = [];
                entry.push({
                    time: ((new Date()).getTime() / 1000),
                    y: value
                });
                rev_chart.push(entry);
                break;
            case 'th':
                $('#td_th').html(Math.round(value / 41));
                $('#progress_th').val(value / 41);
                var entry = [];
                entry.push({
                    time: ((new Date()).getTime() / 1000),
                    y: value
                });
                throttle_chart.push(entry);
                break;
            case 'oil_temp':
                var B = 3400;
                var lnR0 = Math.log(1.981);
                var denom = Math.log(value / 1000.0) - lnR0 + (B / 301.0);
                var oil_temp = B / denom - 273.0;
                $('#progress_oil_temp').val(oil_temp);
                $('#td_oil_temp').html(oil_temp);
                var entry = [];
                entry.push({
                    time: ((new Date()).getTime() / 1000),
                    y: oil_temp
                });
                oil_temp_chart.push(entry);

                break;
            case 'is_fuel_cut':
                if (value == 0) {
                    $('#td_fuel_cut').html("False");
                } else {
                    $('#td_fuel_cut').html("True");
                }
                break;
            case 'is_af_rich':
                if (value == 0) {
                    $('#td_af').html("Lean");
                } else {
                    $('#td_af').html("Rich");
                }
                break;
            case 'vattery_voltage':
                var vattery_voltage = value / 1000.0;
                $('#td_vattery_voltage').html(vattery_voltage);
                $('#progress_vattery_voltage').val(vattery_voltage);
                var entry = [];
                entry.push({
                    time: ((new Date()).getTime() / 1000),
                    y: vattery_voltage
                });
                voltage_chart.push(entry);
                break;
        }
    };

    var car_add_data = function(data) {
        if (data === undefined || data === null) {
            return;
        }

        switch (data.id) {
            case 'car_data':
            case 'engine_data':
                _.forEach(data.value, function(value, prop) {
                    data_apply(prop, value, data.timestamp.microsec + 1000000 * data.timestamp.sec);
                });
                break;
        }
    };

    window.car_add_data = car_add_data;
});
