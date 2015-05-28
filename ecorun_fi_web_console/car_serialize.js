var mongoose = require('mongoose');
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
        if (!CarDatabase.Models.hasOwnProperty(id)) {
            throw new Error(id + " can't serialize");
        }
        var model = new CarDatabase.Models[id](data);
        model.save();
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
        }),
        basic_inject_time_map: new mongoose.Schema({
            value: Buffer,
        })
    };
    CarDatabase.Models = {};
    return CarDatabase;
})();
exports.CarDatabase = CarDatabase;
//# sourceMappingURL=car_serialize.js.map