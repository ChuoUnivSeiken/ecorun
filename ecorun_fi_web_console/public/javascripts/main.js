/// <reference path="./typings/tsd.d.ts"/>
var $ = require('jquery');
global.jQuery = require("jquery");
global.$ = $;
var car_visualize = require("./car_visualize");
var car_access = require("./car_access");
var _ = require('lodash');
(function () {
    var bootstrap = require('bootstrap');
    $(window).ready(function () {
        car_access.initialize();
        car_visualize.initialize();
        var num_row = 16;
        var num_col = 16;
        for (var i = 0; i < num_row; i++) {
            var id_row = 'basic_inject_time_matrix_' + i.toString();
            $('#basic_inject_time_matrix').append('<div width="900px" class="btn-group-lg"" id=' + id_row + '>' + '</div>');
            for (var j = 0; j < num_col; j++) {
                var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
                var clazz = 'btn btn-default';
                var elem = $('#' + id_row);
                elem.append('<button type="button" style="margin: 2px;" class="' + clazz + '" id=' + id_cell + ' data-target-cell=' + id_cell + ' data-popover=popover data-tooltip="tooltip" data-toggle="modal" data-target="#time-setting-modal" data-placement="left" title="Hooray!">' + '0.0' + '</button>');
            }
        }
        var model = $('#time-setting-modal');
        model.on('show.bs.modal', function (e) {
            var target_cell = $(e.relatedTarget).attr('id');
            var time = parseFloat($(e.relatedTarget).html());
            $(e.currentTarget).data('target-cell', target_cell);
            $('#time-view').html(time.toFixed(1).toString() + "ms");
            $('#time-input').val(time);
        });
        model.on('hidden.bs.modal', function (e) {
            var target_cell = $(e.currentTarget).data('target-cell');
            var time = parseFloat($('#time-input').val());
            $('#' + target_cell).html(time.toFixed(1));
        });
        $('#time-input').on('input', function (e) {
            var time = parseFloat($(e.currentTarget).val());
            $('#time-view').html(time.toFixed(1) + 'ms');
        });
        $("#update_basic_inject_time").click(function () {
            car_access.AppConnection.get_instance().request_data('basic_inject_time_map');
        });
        $("#apply_basic_inject_time").click(function () {
            var array = new Uint8Array(256);
            var num_row = 16;
            var num_col = 16;
            for (var i = 0; i < num_row; i++) {
                for (var j = 0; j < num_col; j++) {
                    var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
                    array[i * num_col + j] = parseInt($('#' + id_cell).html());
                }
            }
            var b64encoded = btoa(_.reduce(array, function (previousValue, currentValue, currentIndex, array) {
                return previousValue + String.fromCharCode(currentValue);
            }, ""));
            car_access.AppConnection.get_instance().send_data('basic_inject_time_map', b64encoded, 256);
        });
        $("[data-popover=popover]").popover({
            trigger: 'manual',
            animate: false,
            html: true,
            placement: 'top',
            template: '<div class="popover" onmouseover="$(this).mouseleave(function() {$(this).hide();});" style="display:block;"><div class="arrow"></div><div class="popover-inner"><h3 class="popover-title"></h3><div class="popover-content"><p></p></div></div></div>'
        }).mouseenter(function (e) {
            $(this).popover('show');
        }).mouseleave(function (e) {
            $(this).popover('hide');
        });
        $("#navbar ul li a[href^='#']").on('click', function (e) {
            e.preventDefault();
            var hash = this.hash;
            $('html, body').animate({
                scrollTop: $(hash).offset().top
            }, 300, function () {
                window.location.hash = hash;
            });
        });
    });
})();
