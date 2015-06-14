
(function () {
    var $ = require('jquery');
    window.jQuery = $;
    var bootstrap = require('bootstrap')

    $(window).ready(function () {
        var car_visualize = require("./car_visualize.js");
        car_visualize();

        var car_access = require("./car_access.js");
        car_access();

        var num_row = 16;
        var num_col = 16;
        for (var i = 0; i < num_row; i++) {
            var id_row = 'basic_inject_time_matrix_' + i.toString();
            $('#basic_inject_time_matrix').append('<div width="900px" class="btn-group-lg" id=' + id_row + '>' + '</div>');
            for (var j = 0; j < num_col; j++) {
                var id_cell = 'basic_inject_time_matrix_' + i.toString() + '-' + j.toString();
                var clazz = 'btn btn-default';
                $('#' + id_row).append('<button type="button" style="margin: 2px;" class="' + clazz + '" id=' + id_cell + ' data-tooltip="tooltip" data-toggle="modal" data-target="#modal-example" data-placement="left" title="Hooray!">' + 'H' + '</div>');
            }
        }

        $('[data-tooltip="tooltip"]').tooltip();
    });
})();
