/// <reference path="./typings/tsd.d.ts"/>

import $ = require('jquery')
import car_charts = require('./car_charts');
import events = require('events');
import _ = require('lodash')

class DataParser extends events.EventEmitter {
    parse(data: Object): Object {
        return this._parse(data);
    }

    private _parse(data) {
        if (data === undefined || data === null) {
            return;
        }

        switch (data.id) {
            case 'car_data':
            case 'engine_data':
                return _.map(data.value, (value, prop) => {
                    var result = this.parse_data(prop, value, data.timestamp.microsec + 1000000 * data.timestamp.sec);
                    this.emit('data', result)
                    return result;
                });
        }
    }

    parse_data(name: string, value, timestamp) {
        switch (name) {
            case 'rev':
                return value;
            case 'th':
                return Math.round(value / 41);
            case 'oil_temp':
                var B = 3400;
                var lnR0 = Math.log(1.981);
                var denom = Math.log(value / 1000.0) - lnR0 + (B / 301.0);
                var oil_temp = Math.round(B / denom - 273.0);

                return oil_temp;
            case 'intake_temp':
                var B = 3400;
                var lnR0 = Math.log(1.981);
                var denom = Math.log(value / 1000.0) - lnR0 + (B / 301.0);
                var intake_temp = Math.round(B / denom - 273.0);

                return intake_temp;
            case 'is_fuel_cut':
                return <boolean>value;
            case 'is_af_rich':
                return <boolean>value;
            case 'wheel_rotation_period':
                var speed = 20;
                return speed;
            case 'vattery_voltage':
                return value / 1000.0;
        }
    }
}

export class CarCharts {
    static speed_chart: car_charts.Chart;
    static voltage_chart: car_charts.Chart;
    static rev_chart: car_charts.Chart;
    static throttle_chart: car_charts.Chart;
    static oil_temp_chart: car_charts.Chart;
    static temp_chart: car_charts.Chart;

    static initialize(options: Object = {}) {
        this.speed_chart = new car_charts.Chart(document.getElementById("chart_speed"), options);
        this.voltage_chart = new car_charts.Chart(document.getElementById("chart_vattery_voltage"), options);
        this.rev_chart = new car_charts.Chart(document.getElementById("chart_rev"), options);
        this.throttle_chart = new car_charts.Chart(document.getElementById("chart_throttle"), options);
        this.oil_temp_chart = new car_charts.Chart(document.getElementById("chart_oil_temp"), options);
        this.temp_chart = new car_charts.Chart(document.getElementById("chart_temp"), options);
    }
}

export function initialize() {
    CarCharts.initialize();
}

export function data_apply(name, value, timestamp) {
    switch (name) {
        case 'rev':
            $('#td_rev').html(value);
            $('#progress_rev').attr('aria-valuenow', value).css('width', value + "%");
            CarCharts.rev_chart.push(value);
            break;
        case 'th':
            var th = Math.round(value / 41);
            $('#td_th').html(th.toString());
            $('#progress_th').attr('aria-valuenow', th).css('width', th + "%");
            CarCharts.throttle_chart.push(th);

            break;
        case 'oil_temp':
            var B = 3400;
            var lnR0 = Math.log(1.981);
            var denom = Math.log(value / 1000.0) - lnR0 + (B / 301.0);
            var oil_temp = Math.round(B / denom - 273.0);
            $('#progress_oil_temp').attr('aria-valuenow', oil_temp).css('width', oil_temp + "%");
            $('#td_oil_temp').html(oil_temp.toString());
            CarCharts.oil_temp_chart.push(oil_temp);

            break;
        case 'intake_temp':
            var B = 3400;
            var lnR0 = Math.log(1.981);
            var denom = Math.log(value / 1000.0) - lnR0 + (B / 301.0);
            var oil_temp = Math.round(B / denom - 273.0);
            $('#progress_temp').attr('aria-valuenow', oil_temp).css('width', oil_temp + "%");
            $('#td_temp').html(oil_temp.toString());
            CarCharts.temp_chart.push(oil_temp);

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
            $('#td_speed').html(speed.toString());
            $('#progress_speed').attr('aria-valuenow', speed).css('width', speed / 80 * 100 + "%");
            CarCharts.speed_chart.push(speed);
            break;
        case 'vattery_voltage':
            var vattery_voltage = value / 1000.0;
            $('#td_vattery_voltage').html(vattery_voltage.toString());
            $('#progress_vattery_voltage').attr('aria-valuenow', vattery_voltage).css('width', vattery_voltage / 15 * 100 + "%");
            CarCharts.voltage_chart.push(vattery_voltage);
            break;
        case 'basic_inject_time_map':
            var array = atob(value);
            var num_row = 16;
            var num_col = 16;
            for (var i = 0; i < num_row; i++) {
                for (var j = 0; j < num_col; j++) {
                    var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
                    $('#' + id_cell).html(array.charCodeAt(i * num_col + j).toFixed(1));
                }
            }
            break;
    }
}

export function car_add_data(data) {
    if (data === undefined || data === null) {
        return;
    }

    switch (data.id) {
        case 'basic_inject_time_map':
        case 'car_data':
        case 'engine_data':
            _.forEach(data.value, function(value, prop) {
                data_apply(prop, value, data.timestamp.microsec + 1000000 * data.timestamp.sec);
            });
            break;
    }
};
