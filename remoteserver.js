var express = require('express');
var app = express();
var io = require('socket.io')(app.listen(8081));

app.use(express.static(__dirname + '/app'));
app.use(express.static(__dirname + '/bower_components'));

app.get('/', function (res) {
  res.sendfile('/serverpage.html');
});
