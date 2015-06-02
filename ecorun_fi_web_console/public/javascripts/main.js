
(function () {
    var $ = require('jquery');
    
    $(window).ready(function () {
        var car_visualize = require("./car_visualize.js");
        car_visualize();
        
        var car_access = require("./car_access.js");
        car_access();
    });
})();