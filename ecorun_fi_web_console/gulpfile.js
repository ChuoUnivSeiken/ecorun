
var gulp = require('gulp');
var browserify = require('browserify');
var source = require("vinyl-source-stream");
//var reactify = require('reactify');

gulp.task('build', function () {
    var bundle = browserify({
        entries: ['./public/javascripts/main.js']
    }).bundle();
    return bundle
.pipe(source('app.js'))
.pipe(gulp.dest('./public/javascripts/build'));
});

