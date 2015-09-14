var fs = require('fs');
var path = require('path');
var webpack = require('webpack');


STATIC_ROOT = __dirname;
JS_ROOT = path.join(STATIC_ROOT, 'js');
BUILD_ROOT = path.join(__dirname, 'build');
NPM_ROOT = path.join(__dirname, 'node_modules');

if (!fs.existsSync(BUILD_ROOT)) {
  fs.mkdirSync(BUILD_ROOT)
}

var proto_entry = {
  index: './index',
};

var config = {
  context: JS_ROOT,
  entry: proto_entry,
  output: {
    path: BUILD_ROOT,
    filename: "[name].js",
  },
  cache: true,
  resolve: {
    extensions: ['', '.es6.js', '.js'],
    modulesDirectories: [
      JS_ROOT,
      NPM_ROOT,
    ],
  },
  alias: {
    'lib/bowser': 'bowser',
  },
  module: {
    loaders: [
      { test: /\.es6\.js$/, loader: 'babel', query: {stage: 0} },
      { test: /\.json$/, loader: 'json-loader' },
      { test: /\.coffee$/, loader: 'coffee-loader' },
    ],
  },
  watchDelay: 0,
  devtool: 'eval'   /// give us source mapping!
}

module.exports = config
