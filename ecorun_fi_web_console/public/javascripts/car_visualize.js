/// <reference path="./typings/tsd.d.ts"/>
var __extends = (this && this.__extends) || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
};
var $ = require('jquery');
var car_charts = require('./car_charts');
var events = require('events');
var _ = require('lodash');
var DataParser = (function (_super) {
    __extends(DataParser, _super);
    function DataParser() {
        _super.apply(this, arguments);
    }
    DataParser.prototype.parse = function (data) {
        return this._parse(data);
    };
    DataParser.prototype._parse = function (data) {
        var _this = this;
        if (data === undefined || data === null) {
            return;
        }
        switch (data.id) {
            case 'car_data':
            case 'engine_data':
                return _.map(data.value, function (value, prop) {
                    var result = _this.parse_data(prop, value, data.timestamp.microsec + 1000000 * data.timestamp.sec);
                    _this.emit('data', result);
                    return result;
                });
        }
    };
    DataParser.prototype.parse_data = function (name, value, timestamp) {
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
                return value;
            case 'is_af_rich':
                return value;
            case 'wheel_rotation_period':
                var speed = 20;
                return speed;
            case 'vattery_voltage':
                return value / 1000.0;
        }
    };
    return DataParser;
})(events.EventEmitter);
var CarCharts = (function () {
    function CarCharts() {
    }
    CarCharts.initialize = function (options) {
        if (options === void 0) { options = {}; }
        this.speed_chart = new car_charts.Chart(document.getElementById("chart_speed"), options);
        this.voltage_chart = new car_charts.Chart(document.getElementById("chart_vattery_voltage"), options);
        this.rev_chart = new car_charts.Chart(document.getElementById("chart_rev"), options);
        this.throttle_chart = new car_charts.Chart(document.getElementById("chart_throttle"), options);
        this.oil_temp_chart = new car_charts.Chart(document.getElementById("chart_oil_temp"), options);
        this.temp_chart = new car_charts.Chart(document.getElementById("chart_temp"), options);
    };
    return CarCharts;
})();
exports.CarCharts = CarCharts;
function initialize() {
    CarCharts.initialize();
}
exports.initialize = initialize;
function data_apply(name, value, timestamp) {
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
            }
            else {
                $('#td_fuel_cut').html("True");
            }
            break;
        case 'is_af_rich':
            if (value == 0) {
                $('#td_af').html("Lean");
            }
            else {
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
            var num_row = 8;
            var num_col = 8;
            for (var i = 0; i < num_row; i++) {
                for (var j = 0; j < num_col; j++) {
                    var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
                    $('#' + id_cell).html(array.charCodeAt(i * num_col + j).toFixed(1));
                }
            }
            break;
    }
}
exports.data_apply = data_apply;
function car_add_data(data) {
    if (data === undefined || data === null) {
        return;
    }
    switch (data.id) {
        case 'basic_inject_time_map':
        case 'car_data':
        case 'engine_data':
            _.forEach(data.value, function (value, prop) {
                data_apply(prop, value, data.timestamp.microsec + 1000000 * data.timestamp.sec);
            });
            break;
    }
}
exports.car_add_data = car_add_data;
;
