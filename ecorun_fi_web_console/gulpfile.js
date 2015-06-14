
var gulp = require('gulp');
var browserify = require('browserify');
var source = require("vinyl-source-stream");
var spawn = require('child_process').spawn;
var nodemon = require('gulp-nodemon');
var open = require('gulp-open');
var uglify = require('gulp-uglify');
var buffer = require('vinyl-buffer');
//var reactify = require('reactify');

gulp.task('build-client', function () {
    var bundle = browserify({
        entries: ['./public/javascripts/main.js']
    }).bundle();
    return bundle
.pipe(source('app.js'))
.pipe(buffer())
.pipe(uglify())
.pipe(gulp.dest('./public/javascripts/build'));
});

var ts = require('gulp-typescript');

gulp.task('build-server', function () {
  var tsResult = gulp.src('*.ts')
    .pipe(ts({
        //out: 'app.js',
        target: 'es5',
        module: 'commonjs'
      }));
  tsResult.js.pipe(gulp.dest('.'));
});

gulp.task('run-server', ['build-server'], function () {
  nodemon({
    script: 'app.js'
  , env: { NODE_ENV: 'development' }
  })
})

gulp.task('open', function(){
    var options = {
    url: 'http://localhost',
    app: 'chrome'
      };
  gulp.src('./public/index.html')
  .pipe(open('', options));
});

gulp.task('watch', ['build-client'], function(){
    var watcher = gulp.watch('./public/javascripts/*.js', ['build-client']);
    watcher.on('change', function(event) {
        console.log('File ' + event.path + ' was ' + event.type + ', running tasks...');
    });
    gulp.watch('./public/javascripts/*.js', ['build-client']);
});

gulp.task("default",['build-client', 'run-server', 'open']);
