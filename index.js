var express = require('express');
var app = express();
var io = require('socket.io')(app.listen(8081));

var serialport = require('serialport');
var SerialPort = serialport.SerialPort;

var MongoClient = require('mongodb').MongoClient;
var assert = require('assert');

var ObjectId = require('mongodb').ObjectID;

app.use(express.static(__dirname + '/app'));
app.use(express.static(__dirname + '/bower_components'));

var router = express.Router();

var serialport = new SerialPort("COM3", {
  baudrate: 9600,
  dataBits: 8,
  parity: 'none',
  stopBits: 1,
  parser: serialport.parsers.readline('\n')
}, true);

app.get('/', function (res) {
  res.sendfile('/index.html');
});

var globalDB;

var url = 'mongodb://54.173.182.65/CE490GroupB';
MongoClient.connect(url, function(err, db) {
  assert.equal(null, err);
  globalDB = db;
});

serialport.on('open', function(){
  serialport.on('data', function(data){
    var obj;
    
    try{
      obj = JSON.parse(data);
      obj.epochtime = Date.now();
      obj.triggered = 0;
    }
    catch(e)
    {
      obj = null;
    }
    if(obj != null && globalDB != undefined){
      insertNodeData(globalDB, obj);
      updateNodeData(globalDB, obj);
    }
    
  });

  io.on('connection', function(socket) {
    socket.on("init", function() {
      findData(globalDB);
      findTriggeredData(globalDB);
    });
    socket.on("cleardata", function() {
      globalDB.collection('TriggeredData').remove();
    });
  });
});



var insertNodeData = function(db, nodedata) {
   db.collection('NodeData').insertOne( nodedata, function(err, result) {
    assert.equal(err, null);
  });
};


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
      }
   });
};

var updateNodeData = function(db, nodedata) {
  db.collection('SingleNodeData').findOne({"id":nodedata.id}, function(err, user){
    if(user)
    {
      //exists
      if(nodedata.time > 0)
      {
        nodedata.triggered = 1;
        nodedata.previoustriggeredtime = nodedata.epochtime;
        insertTriggeredData(globalDB, nodedata);
        io.emit('triggereddata', nodedata);
      }
      else if((Date.now() - user.previoustriggeredtime) < 5000)
      {
        nodedata.triggered = 1;
        nodedata.previoustriggeredtime = user.previoustriggeredtime;
      }

      // put in "does not exist" condition as well
      io.emit('data', nodedata);

      db.collection('SingleNodeData').updateOne(
        {"id":nodedata.id},
        {
          $set: 
          {
            "epochtime":nodedata.epochtime,
            "lat": nodedata.lat,
            "lon": nodedata.lon,
            "time": nodedata.time,
            "triggered": nodedata.triggered,
            "previoustriggeredtime": nodedata.previoustriggeredtime
          }
      });
    }
    else
    {
      //does not exist
      db.collection('SingleNodeData').insertOne( nodedata, function(err, result) {
        assert.equal(err, null);
      });
    }
  });
};

var insertTriggeredData = function(db, nodedata) {
   db.collection('TriggeredData').insertOne( nodedata, function(err, result) {
    assert.equal(err, null);
  });
};