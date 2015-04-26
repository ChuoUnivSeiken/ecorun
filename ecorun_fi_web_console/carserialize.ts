
import mongoose = require('mongoose');

class Data {
    timestamp: Date;
}
class EngineData extends Data {
    rev: number = 0;
    is_fuel_cut: number = 0;
    is_af_rich: number = 0;
    th: number = 0;
    oil_temp: number = 0;
    current_total_injected_time: number = 0;
    current_inject_started_count: number = 0;
    current_inject_ended_count: number = 0;
}

export class CarDatabase {
    private static Schemes = {
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

    private static Models: { [key: string]: mongoose.Model<mongoose.Document>; } = {}

    public static connect() {
        mongoose.connect('mongodb://localhost/cardb');
        for (var name in CarDatabase.Schemes) {
            CarDatabase.Models[name] = mongoose.model(name + '_coll', CarDatabase.Schemes[name]);
        }

    }

    public static saveData(id: string, data: Object): void {
        var model = new CarDatabase.Models[id](data);
        model.save();
        console.log(data)
    }

    public static disconnect() {
        mongoose.disconnect();
    }
}
