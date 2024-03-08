import * as THREE from 'three'
import Stats from './jsm/libs/stats.module.js'
import { GUI } from './jsm/libs/lil-gui.module.min.js'
import * as draw from './draw.js'

const stats = Stats()
//document.body.appendChild(stats.dom)

/*const gui = new GUI()
const cameraFolder = gui.addFolder('Camera')
cameraFolder.add(camera.position, 'z', -100, 100)
cameraFolder.open()*/

function animate() {
    requestAnimationFrame(animate);
    draw.update();
    draw.render();
    stats.update();
}

// Create WebSocket connection.
//console.log(`ws://${window.location.hostname}:41235`);
const socket = new WebSocket(`ws://${window.location.hostname}:41235`);
//const socket = new WebSocket(`wss://${window.location.hostname}:443`);

// Connection opened
socket.addEventListener("open", (event) => {
  socket.send("get_status");
});

// Listen for messages
socket.addEventListener("message", async (event) => {
	//console.log("Message from server ", event.data);
	const words = String(event.data).split(' ');
	if(words[0] == "clear") {
		draw.clear();
	} else if(words[0] == "data") {
		words.shift();
		let num = parseInt(words[0]);
		let num_seg = parseInt(words[1]);
		let num_point = parseInt(words[2]);
		let pen_pos = [parseFloat(words[3]), parseFloat(words[4]), parseFloat(words[5])];
		let head_pos = [parseFloat(words[6]), parseFloat(words[7]), parseFloat(words[8])];
		draw.get_data(num, num_seg, num_point, pen_pos, head_pos);
	} else if(words[0] == "status") {
		words.shift();
		draw.get_status(words);
	}
})

draw.init();
animate();


