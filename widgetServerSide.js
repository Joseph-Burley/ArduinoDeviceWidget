var http = require('http');
var url = require('url');
var fs = require('fs');

http.createServer(function(req, res){
	var q = url.parse(req.url,true);
	var path = q.pathname;
	console.log('----------------------------------------');
	console.log(q);
	if(path == '/') //if path is empty, return test.html
	{
		console.log("sending test.html");
		res.writeHead(200, {'content-type':'text/html'});
		var file = fs.readFileSync('./test.html');
		res.write(file);
		res.end();
	}
	else if(path == '/api/getDevice')
	{
		//?device should be an array of device json files
		var devString = q.query.device;
		console.log('devString is:\n' + devString);
		console.log('devString is a: ' + (typeof devString));
		var devArray = JSON.parse(devString);
		console.log('devArray is:\n' + devArray);
		console.log('devArray is a: ' + (typeof devArray));
		
		res.writeHead(200, {'content-type':'text/html'});
		responseArray = [];
		for(var i=0; i<devArray.length; i++)
		{
			fileName = devArray[i];
			console.log("fileName is: " + fileName);
			var filePath = './widgetDevices/' + fileName;
			console.log("the path is: " + filePath);
			if(fs.existsSync(filePath))
				{
			
					var data = fs.readFileSync(filePath);
					data = JSON.parse(data);
					data = JSON.stringify(data);
					responseArray.push(data);
				}			
		}
		console.log(responseArray);
		res.write(JSON.stringify(responseArray));
		res.end();
	}
	else if(path == '/api/devList')
	{
		var filePath = './widgetDevices/';
		var dev_list = fs.readdirSync(filePath);
		res.writeHead(200, {'content-type':'text/html'});
		res.write(JSON.stringify(dev_list));
		res.end();
	}
	else
	{
		res.writeHead(404, {'content-type':'text/html'});
		res.write('<p>sorry</p>');
		res.end();
	}
}).listen(8080);

http.createServer(function(req, res){
	var q = url.parse(req.url, true);
	var path = q.pathname;
	
	if(path == "/api/update")
	{
		
	}
	else if(path == "/api/newDevice")
	{
		
	}
	else if(path == "/api/getCommand")
	{
		
	}
	var s = q.query.j; //j is the querry key that holds the json string
	console.log("The request is: \n" + q);
	console.log("The search j key is: \n" + s);
	var obj;
	try{ //instead of using search, use the query key to extract the JSON string
		obj = JSON.parse(s);
		console.log(obj);
	} catch (e) {
		res.writeHead(400, {'Content-Type':'text/html'});
		res.write("<p>no valid JSON string<p>");
		res.end();
	}
	
	try{
		var filePath = './widgetDevices/' + obj.device + '.json';
		console.log("the path is: " + filePath);
		fs.writeFileSync(filePath, JSON.stringify(obj));
	} catch (e) {
		res.writeHead(404, {'Content-Type':'text/html'});
		res.end();
	}
	
	res.writeHead(200, {'Content-Type':'text/html'});
	res.end();
	
}).listen(8181);