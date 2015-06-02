module.exports = function () {
    var options = {
    }
    
    var car_charts = require("./car_charts.js");

    var speed_chart = new car_charts.Chart(document.getElementById("chart_speed"), options);
    var voltage_chart = new car_charts.Chart(document.getElementById("chart_vattery_voltage"), options);
    var rev_chart = new car_charts.Chart(document.getElementById("chart_rev"), options);
    var throttle_chart = new car_charts.Chart(document.getElementById("chart_throttle"), options);
    var oil_temp_chart = new car_charts.Chart(document.getElementById("chart_oil_temp"), options);
    var temp_chart = new car_charts.Chart(document.getElementById("chart_temp"), options);
    
    var data_apply = function(name, value, timestamp) {
        switch (name) {
            case 'rev':
                $('#td_rev').html(value);
                $('#progress_rev').attr('aria-valuenow', value).css('width', value + "%");
                rev_chart.push(value);
                break;
            case 'th':
                var th = Math.round(value / 41);
                $('#td_th').html(th);
                $('#progress_th').attr('aria-valuenow', th).css('width', th + "%");
                throttle_chart.push(th);

                break;
            case 'oil_temp':
                var B = 3400;
                var lnR0 = Math.log(1.981);
                var denom = Math.log(value / 1000.0) - lnR0 + (B / 301.0);
                var oil_temp = Math.round(B / denom - 273.0);
                $('#progress_oil_temp').attr('aria-valuenow', oil_temp).css('width', oil_temp + "%");
                $('#td_oil_temp').html(oil_temp);
                oil_temp_chart.push(oil_temp);

                break;
            case 'intake_temp':
                var B = 3400;
                var lnR0 = Math.log(1.981);
                var denom = Math.log(value / 1000.0) - lnR0 + (B / 301.0);
                var oil_temp = Math.round(B / denom - 273.0);
                $('#progress_temp').attr('aria-valuenow', oil_temp).css('width', oil_temp + "%");
                $('#td_temp').html(oil_temp);
                temp_chart.push(oil_temp);
                
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
            case 'wheel_rotation_period':
                var speed = 20;
                $('#td_speed').html(speed);
                $('#progress_speed').attr('aria-valuenow', speed).css('width', speed / 80 * 100 + "%");
                speed_chart.push(speed);
                break;
            case 'vattery_voltage':
                var vattery_voltage = value / 1000.0;
                $('#td_vattery_voltage').html(vattery_voltage);
                $('#progress_vattery_voltage').attr('aria-valuenow', vattery_voltage).css('width', vattery_voltage / 15 * 100 + "%");
                voltage_chart.push(vattery_voltage);
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
};
