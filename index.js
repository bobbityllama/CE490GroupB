var express = require('express');
var app = express();
var io = require('socket.io')(app.listen(8081));

var serialport = require('serialport');
var SerialPort = serialport.SerialPort;

app.use(express.static(__dirname + '/app'));

var serialport = new SerialPort("COM3", {
  baudrate: 9600,
  dataBits: 8,
  parity: 'none',
  stopBits: 1,
  parser: serialport.parsers.readline('**')
}, true);

app.get('/', function (res) {
  res.sendfile('/index.html');
});

serialport.on('open', function(){
  console.log('Serial Port Opened');
  var lastValue;

  serialport.on('data', function(data){
    if(lastValue != data.toString())
    {
      if(data.toString().indexOf("HIGH") > -1)
      {
        io.emit('high');
      }
      else
      {
        io.emit('low');
      }
    }
    lastValue = data.toString(); 
  })

  io.on('connection', function(socket) {
      socket.on('lightoff', function() {
        serialport.write("99");
      });
      socket.on('lighton', function() {
        serialport.write("11");
      });
  });
});
