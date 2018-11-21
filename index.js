const http = require('http');
var fs = require('fs');
var request = require('request');

const hostname = '127.0.0.1';
const port = 3000;

fs.readFile('./Operativos/index.html', function (err, html) {
  if (err) throw err;
  http.createServer( function(req,res) {
    res.writeHeader(200, {"Content-Type": "text/html"});
    res.write(html);
    res.end();
  }).listen(port);
});
//const server = http.createServer((req,res) => {
//  res.statusCode = 200;
//  res.setHeader('Content-Type', 'text/plain');
//  res.end('Hello World\n');
//});

//server.listen(port, hostname, () => {
  //console.log(`Server running at http://${hostname}:${port}/`);
//});
