// https://github.com/mrdoob/three.js/blob/master/examples/webgl_lines_fat.html

import * as THREE from 'three'
import { OrbitControls } from './jsm/controls/OrbitControls.js'

import { Line2 } from './jsm/lines/Line2.js';
import { LineMaterial } from './jsm/lines/LineMaterial.js';
import { LineGeometry } from './jsm/lines/LineGeometry.js';
import * as GeometryUtils from './jsm/utils/GeometryUtils.js';

// post processing:
import { EffectComposer } from './jsm/postprocessing/EffectComposer.js';
import { RenderPass } from './jsm/postprocessing/RenderPass.js';
import { ShaderPass } from './jsm/postprocessing/ShaderPass.js';

import { LuminosityShader } from './jsm/shaders/LuminosityShader.js';
import { SobelOperatorShader } from './jsm/shaders/SobelOperatorShader.js';

const segments= [[], []];
const pen_position = [[0, 0, 0], [0, 0, 0]];
const head_position = [[0, 0, 0], [0, 0, 0]];
const lines = [[], []];
const update_requested = [new Set(), new Set()];
let pen = Array(2);
let head = Array(2);
const room_size = [7, 7, 3.5];
let line1;
let matLine, matLineBasic, matLineDashed;
let geometry;
let line = false;
// viewport
let insetWidth;
let insetHeight;
let floor_zscale = 0.5;

const scene = new THREE.Scene();

const camera = new THREE.PerspectiveCamera(40, window.innerWidth / window.innerHeight, 0.1, 1000);
camera.position.set( 0, 2, 4 );

const renderer = new THREE.WebGLRenderer({antialias: true /*, alpha: true*/});
renderer.setSize(window.innerWidth, window.innerHeight);
renderer.setPixelRatio( window.devicePixelRatio );
renderer.setClearColor( 0x000000, 0.0 );
document.body.appendChild(renderer.domElement);

const controls = new OrbitControls(camera, renderer.domElement);
controls.enableDamping = true;
controls.maxDistance = 20;
controls.maxAzimuthAngle = Math.PI / 2.0;
controls.minAzimuthAngle = -Math.PI / 2.0;
controls.minPolarAngle = Math.PI / 6.0;
controls.maxPolarAngle = 2.5 * Math.PI / 4.0;
controls.target.set(0, 1.5, -3);
controls.maxTargetRadius = 20;
controls.update();

window.addEventListener(
    'resize',
    function () {
        camera.aspect = window.innerWidth / window.innerHeight
        camera.updateProjectionMatrix()
        renderer.setSize(window.innerWidth, window.innerHeight)
        render()
    },
    false
)

let composer, effectSobel, finalcomposer;

function setup_post(camera) {
	// postprocessing
	composer = new EffectComposer( renderer );
	const renderPass = new RenderPass( scene, camera );
	composer.addPass( renderPass );

	// color to grayscale conversion

	const effectGrayScale = new ShaderPass( LuminosityShader );
	//composer.addPass( effectGrayScale );

	// you might want to use a gaussian blur filter before
	// the next pass to improve the result of the Sobel operator

	// Sobel operator

	effectSobel = new ShaderPass( SobelOperatorShader );
	effectSobel.uniforms[ 'resolution' ].value.x = window.innerWidth * window.devicePixelRatio;
	effectSobel.uniforms[ 'resolution' ].value.y = window.innerHeight * window.devicePixelRatio;
	composer.addPass( effectSobel );

	/*const renderPass2 = new RenderPass( scene, camera );
	const finalPass = new ShaderPass(
		new THREE.ShaderMaterial( {
			uniforms: {
			baseTexture: { value: null },
			bloomTexture: { value: composer.renderTarget2.texture }
			},
			vertexShader: document.getElementById( 'vertexshader' ).textContent,
			fragmentShader: document.getElementById( 'fragmentshader' ).textContent,
			defines: {}
			} ), "baseTexture"
	);
	finalPass.needsSwap = true;
	finalcomposer = new EffectComposer( renderer );
	finalcomposer.addPass( renderPass2 );
	finalcomposer.addPass( finalPass );*/
}

export function init(/*_renderer, _scene, camera*/) {
	/*renderer = _renderer;
	scene = _scene;*/

	geometry = new LineGeometry();
	matLine = new LineMaterial( {

		color: 0xffffff,
		linewidth: 0.035, // in world units with size attenuation, pixels otherwise
		worldUnits: true,
		vertexColors: true,

		//resolution:  // to be set by renderer, eventually
		dashed: false,
		alphaToCoverage: true,

	} );

	//window.addEventListener( 'resize', onWindowResize );
	//onWindowResize();
	const plane_geom = new THREE.PlaneGeometry();
	const plane_mat = new THREE.MeshBasicMaterial({
		color: 0x404040,
		transparent: true,
		opacity: 0.8,
		wireframe: false,
		//side: THREE.DoubleSide, 
	})
	// front
	let plane = new THREE.Mesh(plane_geom, plane_mat);
	plane.scale.set(room_size[0], room_size[2]);
	plane.position.set(0, room_size[2] / 2.0, -room_size[1] / 2.0);
	scene.add(plane);

	plane = new THREE.GridHelper(1, 10);
	plane.scale.set(room_size[0], 1, room_size[2]);
	plane.position.set(0, room_size[2] / 2.0, -room_size[1] / 2.0);
	plane.rotateX(Math.PI / 2.0);
	scene.add(plane);

	// floor
	plane = new THREE.Mesh(plane_geom, plane_mat);
	plane.scale.set(room_size[0], room_size[1] * floor_zscale);
	plane.position.set(0, 0, -room_size[1] * (1.0 - floor_zscale) / 2.0);
	plane.rotateX(-Math.PI / 2.0);
	scene.add(plane);

	plane = new THREE.GridHelper(1, 10);
	plane.scale.set(room_size[0], 1, room_size[1] * floor_zscale);
	plane.position.set(0, 0, -room_size[1] * (1.0 - floor_zscale) / 2.0);
	//plane.rotateX(Math.PI / 2.0);
	scene.add(plane);

	// left
	plane = new THREE.Mesh(plane_geom, plane_mat);
	plane.scale.set(7, 3.5);
	plane.position.set(-3.5, 1.75, 0);
	plane.rotateY(Math.PI / 2.0);
	//scene.add(plane);



	const sphere_geom = new THREE.SphereGeometry(1, 8, 8);
	const sphere_mat = Array(2);
	sphere_mat[0] = new THREE.MeshBasicMaterial({
		color: 0xF07090,
		//transparent: true,
		//opacity: 0.5,
		wireframe: true,
	});
	sphere_mat[1] = new THREE.MeshBasicMaterial({
		color: 0x7070F0,
		//transparent: true,
		//opacity: 0.5,
		wireframe: true,
	});

	let pen_size = 0.1;
	let head_size = 0.2;
	for(let i = 0; i < 2; i++) {
		pen[i] = new THREE.Mesh(sphere_geom, sphere_mat[i]);
		pen[i].scale.set(pen_size, pen_size, pen_size);
		scene.add(pen[i]);
		head[i] = new THREE.Mesh(sphere_geom, sphere_mat[i]);
		head[i].scale.set(head_size, head_size, head_size);
		scene.add(head[i]);
	}
	//setup_post(camera);
	
	/*const ambient = new THREE.HemisphereLight( 0xffffff, 0x8d8d8d, 0.5 );
	scene.add( ambient );
	renderer.shadowMap.enabled = true;*/
}

export function render()
{
	renderer.setClearColor( 0x000000, 0 );
	//renderer.setClearColor( 0xffffff, 0.5 );
	renderer.setViewport( 0, 0, window.innerWidth, window.innerHeight );

	// renderer will set this eventually
	matLine.resolution.set( window.innerWidth, window.innerHeight ); // resolution of the viewport
	
	controls.target.clamp(
		new THREE.Vector3(-room_size[0] / 2.0, 0.0, -room_size[1] / 2.0),
		new THREE.Vector3(room_size[0] / 2.0, room_size[2], (floor_zscale - 0.5) * room_size[1])
	);
	controls.update();
	renderer.render(scene, camera);
	//composer.render();
	//finalcomposer.render();
}

export function get_data(num, num_seg, num_point, pen_pos, head_pos)
{
	pen[num].position.set(pen_pos[0], pen_pos[1], -pen_pos[2]);
	head[num].position.set(head_pos[0], head_pos[1], -head_pos[2]);
	if(num_point >= 0) {
		if(segments[num][num_seg] == undefined) segments[num][num_seg] = [];
		segments[num][num_seg][num_point] = pen_pos;
		//console.log(`pen ${num} segment ${num_seg} point ${num_point}: ${pen_pos}`);
		update_requested[num].add(num_seg);
	}
}

export function get_status(words)
{
	let roomsize = words[0].split(',');
	//room_size.set(parseFloat(roomsize[0]), parseFloat(roomsize[1]), parseFloat(roomsize[2]));
	let segs0 = parseInt(words[1]);
	let segs1 = parseInt(words[2]);
	let ppos = words[3].split(',');
	let hpos = words[4].split(',');
	pen[0].position.set(parseFloat(ppos[0]), parseFloat(ppos[1]), -parseFloat(ppos[2]));
	pen[1].position.set(parseFloat(ppos[3]), parseFloat(ppos[4]), -parseFloat(ppos[5]));
	head[0].position.set(parseFloat(hpos[0]), parseFloat(hpos[1]), -parseFloat(hpos[2]));
	head[1].position.set(parseFloat(hpos[3]), parseFloat(hpos[4]), -parseFloat(hpos[5]));
	console.log(`status: room_size=${room_size} segs0=${segs0} segs1=${segs1}`);
	request_update(0, segs0);
	request_update(1, segs1);
}

export async function request_update(num, num_seg){
	for(let i = 0; i <= num_seg; i++) {
		//console.log(`Fetching ${num} ${i}`);
		await request_segment(num, i);
	}
	console.log(`All segments fetched for pen ${num}`);
}

function request_segment(num, num_seg) {
	return fetch(`/seg${num}/${num_seg}` , {method: 'GET'})
	.then((response) => {
		if (!response.ok) {
			throw new Error(`HTTP error! Status: ${response.status}`);
		}
		return response.text();
	})
	.then((response) => {
		//console.log(response);
		//console.log(`got seg ${num} ${num_seg}`);
		let words = response.split(" ");
		let command = words.shift();
		if(command == "segment") {
			let num = parseInt(words.shift());
			let num_seg = parseInt(words.shift());
			if(segments[num][num_seg] == undefined) segments[num][num_seg] = [];
			for(let i = 0; i < words.length; i++) {
				let coords = words[i].split(",");
				segments[num][num_seg][i] = [parseFloat(coords[0]), parseFloat(coords[1]), parseFloat(coords[2])];
			}
			//console.log(`pen ${num} segment ${num_seg} point ${num_point}: ${pen_pos}`);
			update_requested[num].add(num_seg);
		}
	});
}

function update_line(num, num_seg) {
	if(lines[num][num_seg] != undefined) {
		scene.remove(lines[num][num_seg]);
		lines[num][num_seg].geometry.dispose();
	}
	const positions = [];
	const colors = [];
	const points = [];
	segments[num][num_seg].forEach(function each(p) {
		if(typeof(p) != 'undefined') {
			//let words = line.split(" ");
			let point = new THREE.Vector3(p[0], p[1], p[2]);
			points.push(point);
		}
	});	
	if(points.length < 4) return;
	const spline = new THREE.CatmullRomCurve3( points );
	const divisions = Math.round( 2 * points.length );
	const point = new THREE.Vector3();
	const color = new THREE.Color();

	//console.log("added ", points.length, " points, divisions: ", divisions);
	for ( let i = 0, l = divisions; i < l; i ++ ) {
		const t = i / l;

		spline.getPoint( t, point );
		positions.push( point.x, point.y , -point.z );

		//color.setHSL( t, 1.0, 0.5, THREE.SRGBColorSpace );
		color.setHSL( 2 * num / 3.0, 0.3, 0.6 + t / 3.0, THREE.SRGBColorSpace );
		colors.push( color.r, color.g, color.b);
	}

	lines[num][num_seg] = new Line2(undefined, matLine );
	lines[num][num_seg].geometry.setPositions( positions );
	lines[num][num_seg].geometry.setColors( colors );

	scene.add(lines[num][num_seg]);
	update_requested[num].delete(num_seg);
}

export function update() {
	for(let num = 0; num < 2; num++) {
		if(update_requested[num].size) {
			let num_seg = update_requested[num].values().next().value;
			update_requested[num].delete(num_seg);
			update_line(num, num_seg);
		}
	}
}

export function clear() {
	for(let num = 0; num < 2; num++) {
		lines[num].forEach(function each(l) {
			scene.remove(l);
			l.geometry.dispose();
		});
		lines[num].length = 0;
		segments[num] = [];
	}
}

