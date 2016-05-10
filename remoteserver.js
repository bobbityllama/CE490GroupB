var express = require('express');
var app = express();
var io = require('socket.io')(app.listen(8081));
var MongoClient = require('mongodb').MongoClient;
var assert = require('assert');

var ObjectId = require('mongodb').ObjectID;

app.use(express.static(__dirname + '/app'));
app.use(express.static(__dirname + '/bower_components'));

var globalDB;

var url = 'mongodb://54.173.182.65/CE490GroupB';
MongoClient.connect(url, function(err, db) {
  assert.equal(null, err);
  globalDB = db;
});

app.get('/', function (res) {
  res.sendfile('/serverpage.html');
});

io.on('connection', function(socket) {
  socket.on('init', function() {
    findData(globalDB);
    findTriggeredData(globalDB);
  });
});

var findData = function(db) {
   var cursor =db.collection('SingleNodeData').find( );
   cursor.each(function(err, doc) {
      assert.equal(err, null);
      if (doc != null) {
         io.emit('data', doc);
      }
   });
};

var findTriggeredData = function(db) {
   var cursor =db.collection('TriggeredData').find( );
   cursor.each(function(err, doc) {
      assert.equal(err, null);
      if (doc != null) {
         io.emit('triggereddata', doc);
         //console.log(doc);
      }
   });
};
