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

var serialport = new SerialPort("COM4", {
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
var deathTime = 10000;
var triggerTime = 5000;
var beaconPeriod = 1000;

var url = 'mongodb://54.173.182.65/CE490GroupB';
MongoClient.connect(url, function(err, db) {
  assert.equal(null, err);
  globalDB = db;
});

serialport.on('open', function(){
  serialport.on('data', function(data){
    var obj;
    console.log(data);
    
    try{
      obj = JSON.parse(data);
    }
    catch(e)
    {
      obj = null;
    }

    if(obj != null && globalDB != undefined){
      obj.sec = obj.sectorID;
      obj.id = obj.deviceID + obj.sectorID;

      if(obj.lat == 0)
      {
        obj.epochtime = Date.now();
        obj.alive = 1;

        insertHeadData(globalDB, obj);
        updateHeadData(globalDB, obj);
      }
      else
      {
        obj.epochtime = Date.now();
        obj.triggered = 0;
        obj.alive = 1;
        obj.previoustriggeredtime = 0;

        insertNodeData(globalDB, obj);
        updateNodeData(globalDB, obj);
      }

      console.log(obj);
    }
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

    socket.on("getheaddata", function() {
      if(globalDB != null)
      {
        findHeadData(globalDB);
      }
    });

    socket.on("cleardata", function() {
      if(globalDB != null)
      {
        globalDB.collection('NodeData').remove();
        globalDB.collection('HeadData').remove();
      }
    });

    socket.on("cleartriggereddata", function() {
      if(globalDB != null)
      {
        globalDB.collection('TriggeredData').remove();
      }
    });

    socket.on("clearsingledata", function() {
      if(globalDB != null)
      {
        globalDB.collection('SingleNodeData').remove();
        globalDB.collection('SingleHeadData').remove();
      }
    });

    socket.on("triggertimesubmit", function(data) {
      if(globalDB != null)
      {
        globalDB.collection('ConfigData').findOne({"name":"TriggerTime"}, function(err, user){
          if(user)
          {
            globalDB.collection('ConfigData').updateOne(
              {"name":"TriggerTime"},
              {$set:{"value":data}}
            );
          }
          else
          {
            var configData = {"name":"TriggerTime", "value":data};

            globalDB.collection('ConfigData').insertOne( configData, function(err, result) {
              assert.equal(err, null);
            });
          }
        });
      }
    });

    socket.on("sensordeathtimesubmit", function(data) {
      if(globalDB != null)
      {
        globalDB.collection('ConfigData').findOne({"name":"SensorDeathTime"}, function(err, user){
          if(user)
          {
            globalDB.collection('ConfigData').updateOne(
              {"name":"SensorDeathTime"},
              {$set:{"value":data}}
            );
          }
          else
          {
            var configData = {"name":"SensorDeathTime", "value":data};

            globalDB.collection('ConfigData').insertOne( configData, function(err, result) {
              assert.equal(err, null);
            });
          }
        });
      }
    });

    socket.on("beaconperiodsubmit", function(data) {
      if(globalDB != null)
      {
        globalDB.collection('ConfigData').findOne({"name":"BeaconPeriod"}, function(err, user){
          if(user)
          {
            globalDB.collection('ConfigData').updateOne(
              {"name":"BeaconPeriod"},
              {$set:{"value":data}}
            );
          }
          else
          {
            var configData = {"name":"BeaconPeriod", "value":data};

            globalDB.collection('ConfigData').insertOne( configData, function(err, result) {
              assert.equal(err, null);
            });
          }
        });
      }

      socket.write("{\"beaconperiod\":\"" + data + "\"}");
    });
  });
});

setInterval(function(){
  if(globalDB != undefined)
  {
    var SDTcursor = globalDB.collection('ConfigData').find({"name":"SensorDeathTime"});
    SDTcursor.each(function(err, doc){
      if(doc != null){
        deathTime = doc.value;
        io.emit('deathTimeUpdate', deathTime);
      }
    });

    var TTcursor = globalDB.collection('ConfigData').find({"name":"TriggerTime"});
    TTcursor.each(function(err, doc){
      if(doc != null){
        triggerTime = doc.value;
        io.emit('triggerTimeUpdate', triggerTime);
      }
    });

    var BPcursor = globalDB.collection('ConfigData').find({"name":"BeaconPeriod"});
    BPcursor.each(function(err, doc){
      if(doc != null){
        beaconPeriod = doc.value;
        io.emit('beaconPeriodUpdate', beaconPeriod);
      }
    });

    var nodeCursor = globalDB.collection('SingleNodeData').find( );
    nodeCursor.each(function(err, doc) {
      assert.equal(err, null);
      if (doc != null) {
        if((Date.now() - doc.epochtime) > deathTime)
        {
          //console.log(Date.now()+" - "+doc.epochtime+"="+(Date.now() - doc.epochtime));
          globalDB.collection('SingleNodeData').updateOne(
            {"id":doc.id},
            {$set:{"alive": 0}}
          );
        }
      }
    });

    var headCursor = globalDB.collection('SingleHeadData').find( );
    headCursor.each(function(err, doc) {
      assert.equal(err, null);
      if (doc != null) {
        if((Date.now() - doc.epochtime) > deathTime)
        {
          //console.log(Date.now()+" - "+doc.epochtime+"="+(Date.now() - doc.epochtime));
          globalDB.collection('SingleHeadData').updateOne(
            {"id":doc.id},
            {$set:{"alive": 0}}
          );
        }
      }
    });
  }
}, 250);

var insertNodeData = function(db, nodedata) {
   db.collection('NodeData').insertOne( nodedata, function(err, result) {
    assert.equal(err, null);
  });
};

var insertHeadData = function(db, headdata) {
   db.collection('HeadData').insertOne( headdata, function(err, result) {
    assert.equal(err, null);
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
      }
      else if((Date.now() - user.previoustriggeredtime) < triggerTime)
      {
        nodedata.triggered = 1;
        nodedata.previoustriggeredtime = user.previoustriggeredtime;
      }

      db.collection('SingleNodeData').updateOne(
        {"id":nodedata.id},
        {
          $set: 
          {
            "sec":nodedata.sec,
            "lat": nodedata.lat,
            "lng": nodedata.lng,
            "time": nodedata.time,
            "epochtime": nodedata.epochtime,
            "triggered": nodedata.triggered,
            "alive": nodedata.alive,
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

var updateHeadData = function(db, headdata) {
  db.collection('SingleHeadData').findOne({"id":headdata.id}, function(err, user){
    if(user)
    {
      //exists
      db.collection('SingleHeadData').updateOne(
        {"id":headdata.id},
        {
          $set:
          {
            "sec":headdata.sec,
            "epochtime": headdata.epochtime,
            "alive": headdata.alive
          }
      });
    }
    else
    {
      //does not exist
      db.collection('SingleHeadData').insertOne( headdata, function(err, result) {
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
   var cursor = db.collection('TriggeredData').find( );
   cursor.each(function(err, doc) {
      assert.equal(err, null);
      if (doc != null) {
         io.emit('triggereddata', doc);
      }
   });
};

var findHeadData = function(db) {
   var cursor = db.collection('SingleHeadData').find( );
   cursor.each(function(err, doc) {
      assert.equal(err, null);
      if (doc != null) {
         io.emit('headdata', doc);
      }
   });
};