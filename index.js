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


router.get('/test', function(req, res, next){
  res.sendfile('/serverpage.html');
});

var url = 'mongodb://54.173.182.65/CE490GroupB';
MongoClient.connect(url, function(err, db) {
  assert.equal(null, err);
  console.log("Connected correctly to server.");
  db.close();
});

var skipped = 0;

serialport.on('open', function(){
  console.log('Serial Port Opened');
  //var lastValue;

  serialport.on('data', function(data){
    //console.log(data.toString());
    var obj;
    
    try{
      obj = JSON.parse(data);
      obj.epochtime = Date.now();
      obj.triggered = 0;
    }
    catch(e)
    {
      console.log("not json");
      obj = null;
    }
    //console.log(obj);
    if(obj != null){
      

      //console.log(obj);

      MongoClient.connect(url, function(err, db) {
        //assert.equal(null, err);
        //console.log(db);
        //console.log(obj);
        //console.log("==================================");
        if(db != null)
        {
          insertNodeData(db, function() {
            db.close();
          }, obj);
        }
        else
        {
          skipped++;
          console.log(skipped);
          db.close();
        }
        
      });

      MongoClient.connect(url, function(err, db) {
        //assert.equal(null, err);
        //console.log(db);
        //console.log(obj);
        //console.log("++++++++++++++++++++++++++++++++++");
        if(db != null)
        {
          updateNodeData(db, function() {
            db.close();
          }, obj);
        }
        else
        {
          skipped++;
          console.log(skipped);
          db.close();
        }
        
      });
    }
    
  });

  io.on('connection', function(socket) {
      socket.on('lightoff', function() {
        //serialport.write("99");
      });
      socket.on('lighton', function() {
        //serialport.write("11");
      });
      
      socket.on("init", function() {
        MongoClient.connect(url, function(err, db) {
          //assert.equal(null, err);
          if(db != null)
          {
            findData(db, function() {
              db.close();
            });
          }
          else
          {
            skipped++;
            console.log(skipped);
          }
        });
      });
  });
});



var insertNodeData = function(db, callback, nodedata) {
   db.collection('NodeData').insertOne( nodedata, function(err, result) {
    assert.equal(err, null);
    //console.log("Inserted a document into the NodeData collection.");
    callback();
  });
};


var findData = function(db, callback) {
   var cursor =db.collection('SingleNodeData').find( );
   cursor.each(function(err, doc) {
      assert.equal(err, null);
      if (doc != null) {
         io.emit('data', doc);
      } else {
         callback();
      }
   });
};

var updateNodeData = function(db, callback, nodedata) {
  db.collection('SingleNodeData').findOne({"id":nodedata.id}, function(err, user){
    if(user)
    {
      //console.log(user);
      if(nodedata.time > 0)
      {
        nodedata.triggered = 1;
        nodedata.previoustriggeredtime = nodedata.epochtime;
      }
      else if((Date.now() - user.previoustriggeredtime) < 1000)
      {
        nodedata.triggered = 1;
        nodedata.previoustriggeredtime = user.previoustriggeredtime;
        //console.log("2");
      }

      //console.log(Date.now() + "/" + nodedata.previoustriggeredtime + "/" + user.previoustriggeredtime);
      //console.log(Date.now() - nodedata.previoustriggeredtime)

      io.emit('data', nodedata);

      //exists
      //console.log(nodedata.id + " exists");
      db.collection('SingleNodeData').updateOne(
        {"id":nodedata.id},
        {
          $set: {"epochtime":nodedata.epochtime},
          $set: { "lat": nodedata.lat },
          $set: { "lon": nodedata.lon },
          $set: { "time": nodedata.time },
          $set: { "triggered": nodedata.triggered },
          $set: { "previoustriggeredtime": nodedata.previoustriggeredtime }
        }
      , function(err, results) {
          //console.log(results);
          //console.log(nodedata);
          callback();
       });
    }
    else
    {
      //does not exist
      //console.log(nodedata.id + " does not exist");
      db.collection('SingleNodeData').insertOne( nodedata, function(err, result) {
        assert.equal(err, null);
        //console.log(nodedata);
        callback();
      });
    }
  });
};
