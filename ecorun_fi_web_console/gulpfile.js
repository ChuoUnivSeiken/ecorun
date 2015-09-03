
var gulp = require('gulp');
var browserify = require('browserify');
var source = require("vinyl-source-stream");
var spawn = require('child_process').spawn;
var nodemon = require('gulp-nodemon');
var open = require('gulp-open');
var uglify = require('gulp-uglify');
var buffer = require('vinyl-buffer');
var plumber = require('gulp-plumber');
var handle_errors = require('./handle_errors.js')
var ts = require('gulp-typescript');
//var reactify = require('reactify');

gulp.task('build-client', function() {
    var bundle = browserify({
        entries: ['./public/javascripts/main.js']
    }).bundle();
    return bundle
        .on('error', handle_errors)
        .pipe(plumber())
        .pipe(source('app.js'))
        //.pipe(buffer())
        //.pipe(uglify())
        .pipe(gulp.dest('./public/javascripts/build'));
});

gulp.task('build-server', function() {
    return gulp.src('*.ts')
        .pipe(ts({
            target: 'es5',
            module: 'commonjs'
        }))
        .js
        .pipe(gulp.dest('.'));
});

gulp.task('run-server', ['build-server'], function() {
    nodemon({
        script: 'bin/www',
        env: {
            NODE_ENV: 'development'
        }
    });
})

gulp.task('watch', ['build-client'], function() {
    var watcher = gulp.watch('./public/javascripts/*.js', ['build-client']);
    watcher.on('change', function(event) {
        console.log('File ' + event.path + ' was ' + event.type + ', running tasks...');
    });
});

gulp.task('open', ['watch'], function() {
    var options = {
        url: 'http://localhost',
        app: 'chrome'
    };
    return gulp.src('./public/index.html')
        .pipe(open('', options));
});

gulp.task("default", ['run-server', 'open']);
