var __extends = this.__extends || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    __.prototype = b.prototype;
    d.prototype = new __();
};
var mongoose = require('mongoose');
var Data = (function () {
    function Data() {
    }
    return Data;
})();
var EngineData = (function (_super) {
    __extends(EngineData, _super);
    function EngineData() {
        _super.apply(this, arguments);
        this.rev = 0;
        this.is_fuel_cut = 0;
        this.is_af_rich = 0;
        this.th = 0;
        this.oil_temp = 0;
        this.current_total_injected_time = 0;
        this.current_inject_started_count = 0;
        this.current_inject_ended_count = 0;
    }
    return EngineData;
})(Data);
var CarDatabase = (function () {
    function CarDatabase() {
    }
    CarDatabase.connect = function () {
        mongoose.connect('mongodb://localhost/cardb');
        for (var name in CarDatabase.Schemes) {
            CarDatabase.Models[name] = mongoose.model(name + '_coll', CarDatabase.Schemes[name]);
        }
    };
    CarDatabase.saveData = function (id, data) {
        var model = new CarDatabase.Models[id](data);
        model.save();
        console.log(data);
    };
    CarDatabase.disconnect = function () {
        mongoose.disconnect();
    };
    CarDatabase.Schemes = {
        engine_data: new mongoose.Schema({
            rev: Number,
            is_fuel_cut: Number,
            is_af_rich: Number,
            th: Number,
            oil_temp: Number,
            current_total_injected_time: Number,
            current_inject_started_count: Number,
            current_inject_ended_count: Number,
            timestamp: Date,
        }),
        car_data: new mongoose.Schema({
            vattery_voltage: Number,
            wheel_count: Number,
            wheel_rotation_period: Number,
        })
    };
    CarDatabase.Models = {};
    return CarDatabase;
})();
exports.CarDatabase = CarDatabase;
//# sourceMappingURL=carserialize.js.map