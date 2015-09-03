/// <reference path="./typings/tsd.d.ts"/>
require('bootstrap');
var car_visualize = require("./car_visualize");
var car_access = require("./car_access");
var _ = require('lodash');
$(document).ready(function () {
    car_access.initialize();
    car_visualize.initialize();
    var rev = [800, 1600, 2400, 3200, 4000, 4800, 5600, 6400];
    var th = [10, 20, 30, 40, 50, 60, 70, 80];
    var num_row = 8;
    var num_col = 8;
    for (var i = 0; i < num_row; i++) {
        var id_row = 'basic_inject_time_matrix_' + i.toString();
        $('#basic_inject_time_matrix').append('<div width="900px" class="btn-group-lg"" id=' + id_row + '>' + '</div>');
        for (var j = 0; j < num_col; j++) {
            var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
            var clazz = 'btn btn-default';
            var elem = $('#' + id_row);
            elem.append('<button type="button" style="margin: 2px;" class="' + clazz + '" id=' + id_cell + ' data-target-cell=' + id_cell + ' data-popover=popover data-tooltip="tooltip" data-toggle="modal" data-target="#time-setting-modal" data-placement="left" title="噴射時間設定">' + '2.5' + '</button>');
            var cell = $('#' + id_cell);
            var rev_beg = rev[j].toString();
            var rev_end = j != num_col - 1 ? rev[j + 1].toString() : " ";
            cell.data('rev', rev_beg + "rpm - " + rev_end + "rpm");
            var th_beg = th[j].toString();
            var th_end = j != num_col - 1 ? th[j + 1].toString() : " ";
            cell.data('th', th_beg + "% - " + th_end + "%");
        }
    }
    var model = $('#time-setting-modal');
    var time_view = $('#time-setting-modal').find('#time-view');
    var time_input = $('#time-setting-modal').find('#time-input');
    model.on('shown.bs.modal', function (e) {
        var target_cell = $(e.relatedTarget).attr('id');
        var time = parseFloat($(e.relatedTarget).html());
        $(e.currentTarget).data('target-cell', target_cell);
        time_view.html(time.toFixed(1) + "ms");
        time_input.val(time.toString());
    });
    time_input.on('input', function (e) {
        var time = parseFloat($(e.currentTarget).val());
        time_view.html(time.toFixed(1) + 'ms');
    });
    model.on('hidden.bs.modal', function (e) {
        var target_cell = $(e.currentTarget).data('target-cell');
        var time = parseFloat(time_input.val());
        $('#' + target_cell).html(time.toFixed(1));
    });
    $("#update_basic_inject_time").click(function () {
        car_access.AppConnection.get_instance().request_data('basic_inject_time_map');
    });
    $("#save_basic_inject_time").click(function () {
        var array = new Uint8Array(num_row * num_col);
        for (var i = 0; i < num_row; i++) {
            for (var j = 0; j < num_col; j++) {
                var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
                array[i * num_col + j] = parseInt($('#' + id_cell).html());
            }
        }
        var b64encoded = btoa(_.reduce(array, function (previousValue, currentValue, currentIndex, array) {
            return previousValue + String.fromCharCode(currentValue);
        }, ""));
        car_access.AppConnection.get_instance().save_data('basic_inject_time_map', b64encoded);
    });
    $("#apply_basic_inject_time").click(function () {
        var array = new Uint8Array(num_row * num_col);
        for (var i = 0; i < num_row; i++) {
            for (var j = 0; j < num_col; j++) {
                var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
                array[i * num_col + j] = (parseFloat($('#' + id_cell).html()) * 20);
            }
        }
        var b64encoded = btoa(_.reduce(array, function (previousValue, currentValue, currentIndex, array) {
            return previousValue + String.fromCharCode(currentValue);
        }, ""));
        car_access.AppConnection.get_instance().send_data('basic_inject_time_map', b64encoded, num_row * num_col);
    });
    $("#navbar ul li a[href^='#']").click(function (e) {
        e.preventDefault();
        var hash = this.hash;
        $('html, body').animate({
            scrollTop: $(hash).offset().top
        }, 300, function () {
            window.location.hash = hash;
        });
    });
    var popover_template = [
        '<div class="popover" onmouseover="$(this).mouseleave(function() {$(this).hide();});" style="display:block;">',
        '<div class="arrow"></div>',
        '<div class="popover-inner">',
        '<h3 class="popover-title"></h3>',
        '<div class="popover-content"><p></p></div>',
        '</div>',
        '</div>'
    ];
    $("[data-popover=popover]").popover({
        trigger: 'manual',
        animate: false,
        html: true,
        placement: 'left',
        template: popover_template.join("")
    }).mouseenter(function (e) {
        var rev_range = $(this).data('rev');
        var th_range = $(this).data('th');
        var val = $(this).html();
        var content_html = [
            '<table class="table">',
            '<tr>',
            '<th>スロットル</th>',
            '<td>' + th_range + '</td>',
            '</tr>',
            '<tr>',
            '<th>回転数</th>',
            '<td>' + rev_range + '</td>',
            '</tr>',
            '<tr>',
            '<th>値</th>',
            '<td>' + val + '</td>',
            '</tr>',
            '</table>'
        ];
        $(this).attr('data-content', content_html.join(""));
        $(this).popover('show');
    }).mouseleave(function (e) {
        $(this).popover('hide');
    });
});
