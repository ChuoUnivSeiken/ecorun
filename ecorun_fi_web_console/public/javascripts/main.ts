/// <reference path="./typings/tsd.d.ts"/>

require('bootstrap')
import car_visualize = require("./car_visualize");
import car_access = require("./car_access");
import _ = require('lodash');

$(document).ready(() => {
    car_access.initialize();
    car_visualize.initialize();

    var rev = [0, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200, 3600, 4000, 4400, 4800, 5200, 5600, 6000];
    var th = [0, 400, 800, 1200, 1600, 2000, 2400, 2800, 3200, 3600, 4000, 4400, 4800, 5200, 5600, 6000];

    var num_row = 16;
    var num_col = 16;
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
            cell.data('rev', rev_beg + " - " + rev_end);

            var th_beg = rev[j].toString();
            var th_end = j != num_col - 1 ? rev[j + 1].toString() : " ";
            cell.data('th', th_beg + " - " + th_end);
        }
    }

    var model = $('#time-setting-modal');
    var time_view = $('#time-setting-modal').find('#time-view');
    var time_input = $('#time-setting-modal').find('#time-input');

    model.on('shown.bs.modal', (e) => {
        var target_cell = $(e.relatedTarget).attr('id');
        var time = parseFloat($(e.relatedTarget).html());
        $(e.currentTarget).data('target-cell', target_cell);
        time_view.html(time.toFixed(1) + "ms");
        time_input.val(time.toString());
    });

    time_input.on('input', (e) => {
        var time = parseFloat($(e.currentTarget).val());
        time_view.html(time.toFixed(1) + 'ms');
    });

    model.on('hidden.bs.modal', (e) => {
        var target_cell = $(e.currentTarget).data('target-cell');
        var time = parseFloat(time_input.val());
        $('#' + target_cell).html(time.toFixed(1));
    });

    $("#update_basic_inject_time").click(() => {
        car_access.AppConnection.get_instance().request_data('basic_inject_time_map');
    });

    $("#save_basic_inject_time").click(() => {
        var array = new Uint8Array(256);
        var num_row = 16;
        var num_col = 16;
        for (var i = 0; i < num_row; i++) {
            for (var j = 0; j < num_col; j++) {
                var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
                array[i * num_col + j] = parseInt($('#' + id_cell).html());
            }
        }
        var b64encoded = btoa(_.reduce(array, (previousValue, currentValue, currentIndex, array) => {
            return previousValue + String.fromCharCode(currentValue);
        }, ""));

        car_access.AppConnection.get_instance().save_data('basic_inject_time_map', b64encoded);
    });

    $("#apply_basic_inject_time").click(() => {
        var array = new Uint8Array(256);
        var num_row = 16;
        var num_col = 16;
        for (var i = 0; i < num_row; i++) {
            for (var j = 0; j < num_col; j++) {
                var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
                array[i * num_col + j] = parseInt($('#' + id_cell).html());
            }
        }
        var b64encoded = btoa(_.reduce(array, (previousValue, currentValue, currentIndex, array) => {
            return previousValue + String.fromCharCode(currentValue);
        }, ""));

        car_access.AppConnection.get_instance().send_data('basic_inject_time_map', b64encoded, 256);
    });

    $("#navbar ul li a[href^='#']").click(function(e) {
        // prevent default anchor click behavior
        e.preventDefault();

        // store hash
        var hash = this.hash;

        // animate
        $('html, body').animate({
            scrollTop: $(hash).offset().top
        }, 300, () => {

                // when done, add hash to url
                // (default click behaviour)
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

    //$('[data-tooltip="tooltip"]').tooltip();
    $("[data-popover=popover]").popover({
        trigger: 'manual',
        animate: false,
        html: true,
        placement: 'left',
        template: popover_template.join("")
    }).mouseenter(function(e) {
        var rev_range = $(this).data('rev') as string;
        var th_range = $(this).data('rev') as string;
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
    }).mouseleave(function(e) {
        $(this).popover('hide');
    });
});
