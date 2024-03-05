const express = require('express')
const app = express()
const path = require('path')
const ws = require('ws')

//const draw_data = [];
const segments= [[], []];
const is_drawing = [0, 0];
const room_size = [7, 7, 3];
const pen_pos = Array(2);
const head_pos = Array(2);
x = 0

app.use(express.static(__dirname + '/public'))
app.use('/build/', express.static(path.join(__dirname, 'node_modules/three/build')))
app.use('/jsm/', express.static(path.join(__dirname, 'node_modules/three/examples/jsm')))

app.listen(3000, () => console.log('Visit http://127.0.0.1:3000'))

/* setup WebSocket */
//import { WebSocketServer } from 'ws';
const wss = new ws.WebSocketServer({ port: 41235 });

wss.on('connection', function connection(ws) {
	ws.on('error', console.error);

	ws.on('message', function message(data) {
		//console.log('received: %s', data);
		if(data == "get_status") {
			ws.send(`status ${room_size} ${segments[0].length} ${segments[1].length} ${pen_pos} ${head_pos}`);
		}
	});
	//ws.send('something');
});

function sendAllClients(message)
{
	wss.clients.forEach(function each(ws) {
		/*if (ws.isAlive === false) return ws.terminate();
		ws.isAlive = false;
		ws.ping();*/
		ws.send(message);
	});
}

/* setup UDP */
const dgram = require('node:dgram');
const udpserver = dgram.createSocket('udp4');

udpserver.on('error', (err) => {
  console.error(`udpserver error:\n${err.stack}`);
  udpserver.close();
});

udpserver.on('message', (msg, rinfo) => {
	//console.log(`udpserver got: ${msg} from ${rinfo.address}:${rinfo.port}`);
	const message = String(msg).slice(0, -2);
	const words = message.split(' ');
	if(words[0] == "draw_clear") {
		//draw_data.length = 0;
		segments[0].length = 0;
		segments[1].length = 0;
		is_drawing[0] = is_drawing[1] = 0;
		sendAllClients("clear");
	}
	else if(words[0] == "data") {
		words.shift(); // remove 'data'
		let num = parseInt(words[0]) - 1;
		let drawing = parseInt(words[4]);
		pen_pos[num] = [parseFloat(words[1]), parseFloat(words[2]), parseFloat(words[3])];
		head_pos[num] = [parseFloat(words[5]), parseFloat(words[6]), parseFloat(words[7])];
		if(is_drawing[num] != drawing) {
			is_drawing[num] = drawing;
			if(drawing) {
				segments[num].push([]);
				console.log("new segment ", segments[num].length, " for pen ", num);
			}
		}
		if(drawing) {
			segments[num][segments[num].length - 1].push(pen_pos[num]);
			/*console.log("pen ", num, " segment ", segments[num].length - 1,
			" add pos num:",  segments[num][segments[num].length - 1].length - 1, ": ", pen_pos);*/
		}
		let num_seg = segments[num].length - 1;
		let num_point = drawing ? segments[num][segments[num].length - 1].length - 1 : -1;
		//let message = String(draw_data.length) + " " + words.join(" ");
		let message = num + " " + num_seg + " " + num_point + " " + pen_pos[num].join(" ") + " " + head_pos[num].join(" ");
		//console.log(words.join(" "));
		//if(parseInt(words[4]) == 1) draw_data.push(message);
		sendAllClients("data " + message);
	}
	/*else if(words[0] == "get_datalen") {
		console.log(`datalen: ${draw_data.length}`);
	}
	else if(words[0] == "get_data") {
		let line = draw_data[parseFloat(words[1])];
		console.log(`data: ${line}`);
	}*/
	else if(words[0] == "get_segment") {
		let pen = parseInt(words[1]);
		let seg = parseInt(words[2]);
		if(pen >= 0 && pen < 2 && seg < segments[pen].length)
			console.log(`pen ${pen} segment ${seg}: ${segments[pen][seg]}`);
	}
	else if(words[0] == "to_clients") {
		words.shift();
		sendAllClients(words.join(" "));
	}
});

udpserver.on('listening', () => {
  const address = udpserver.address();
  console.log(`udpserver listening ${address.address}:${address.port}`);
});

udpserver.bind(31234);
// Prints: udpserver listening 0.0.0.0:31234



/* setup GET routing */
//const router = express.Router();
app.get("/truc/*", function (req, res) {
  console.log(`Got a GET request ${path.basename(req.url)}`);
  res.send(`Réponse à la requête GET. x = ${x}`);
});

for(let num = 0; num < 2; num++) {
	app.get(`/seg${num}/*`, function (req, res) {
		let num_seg = parseInt(path.basename(req.url));
		//console.log(`Requested pen ${num} segment ${num_seg}`);
		if(num_seg < segments[num].length)
			res.send(`segment ${num} ${num_seg} ${segments[num][num_seg].join(" ")}`);
		else res.send(`none`);
	});
}

