var express = require('express');
var app = express();
var io = require('socket.io')(app.listen(8081));

var MongoClient = require('mongodb').MongoClient;
var assert = require('assert');

var ObjectId = require('mongodb').ObjectID;

app.use(express.static(__dirname + '/remoteapp'));
app.use(express.static(__dirname + '/bower_components'));

var router = express.Router();

app.get('/', function (res) {
  res.sendfile('/index.html');
});

var globalDB;

var url = 'mongodb://54.173.182.65/CE490GroupB';
MongoClient.connect(url, function(err, db) {
  assert.equal(null, err);
  globalDB = db;
});

io.on('connection', function(socket) {
  socket.on("getdata", function() {
    if(globalDB != null)
    {
      findData(globalDB);
    }
  });

  socket.on("gettriggereddata", function() {
    if(globalDB != null)
    {
      findTriggeredData(globalDB);
    }
  });
});

var findData = function(db) {
   var cursor = db.collection('SingleNodeData').find( );
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
      }
   });
};