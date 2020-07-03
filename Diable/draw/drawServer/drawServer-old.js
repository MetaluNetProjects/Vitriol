let Net = require('net'),
	fs=require('fs'),
	path = require('path');

let argv = require('minimist')(process.argv.slice(2));


let DRAW_PORT = argv['port'] || 8080,
	DATA_PATH=path.join(__dirname,"data"),
	CLONE =argv['clone'] || false,
	SAVEDATA=argv['savemove'] ||false,
	SAVEDRAW=argv['savedraw'] ||false,
	SENDDRAW=argv['senddraw'] ||false;

if( SAVEDATA ) console.log("data will be saved in data folder");
if( SAVEDRAW ) console.log("draw wiil be saved in data folder");
if( SENDDRAW ) console.log("draw will be send to other user")
if ( CLONE ) console.log("user position will be send to itself");

let fileOne=null, fileTwo=null,fileDrawOne=null,fileDrawTwo=null;
let fileOneOpen=false, fileTwoOpen=false,fileDrawOneOpen=false,fileDrawTwoOpen=false;

let OneStartDraw=false;
let TwoStartDraw=false;
if (!fs.existsSync(DATA_PATH)){			//si le répertoire n'existe pas, on le crée
    fs.mkdirSync(DATA_PATH);
}


/****************** DRAW SERVER ********************/
const clientDraw = Net.Server();

clientDraw.listen(DRAW_PORT,function(){
		console.log("Drawing server start on : "+DRAW_PORT);
});

let ClientOne=null,
	ClientTwo=null,
	ClientThree=null;

let tick=0;
clientDraw.on('connection', function(socket) {

	console.log('Un client connecté');

	socket.on('data',function(data){
		if(socket != ClientOne && socket!=ClientTwo){
			let datas=data.toString().split(';');
			if(datas[0]=="\\headsetId")
			{
				if(datas[1] == "1"){
					ClientOne = socket;
					console.log("headset 1 connected");
					if(SAVEDATA)createDataFile(1);
					if(SENDDRAW && !SAVEDRAW){
						socket.write("\\senddraw;0;");
						socket.write("\\affichedraw;0;");
					}
					else if(SAVEDRAW)createDrawFile(1);
				} 
				else if (datas[1] == "2") {
					ClientTwo = socket;
					console.log("headset 2 connected");
					if(SAVEDATA)createDataFile(2);
					if(SENDDRAW && !SAVEDRAW){
						socket.write("\\senddraw;0;");
						socket.write("\\affichedraw;0;");
					}
					else if(SAVEDRAW)createDrawFile(2);
				}
				else if (datas[1] == "3") {
					ClientThree = socket;
					console.log("client 3 connected");
				}
				if(CLONE) socket.write("\\clone;0;");
			}
		}else{
			sendData(socket,data);
			gestionPosition(socket,data)
			if(SAVEDATA || SAVEDRAW) gestionData(socket,data);
		}
	});


	socket.on('end',function(err){
		if(socket == ClientOne){
				console.log("headset 1 disconnected");
				if(fileOne!=null)fileOne.end();
				if(fileDrawOne!=null)fileDrawOne.end();
				fileDrawOneOpen=false;
				fileOneOpen=false;
				ClientOne =null;
		}else if(socket == ClientTwo ){
				console.log("headset 2 disconnected");
				if(fileTwoOpen)fileTwo.end();
				if(fileDrawTwo!=null)fileDrawOne.end();
				fileDrawTwoOpen=false;
				fileTwoOpen=false;
				ClientTwo =null;
		}else if(socket == ClientThree ){
				console.log("client 3 disconnected");
				ClientThree =null;
		}else{
			console.log("Client sans Id disconnected")
		}
	});

	socket.on('error',function(err){
		if(socket == ClientOne){
				console.log("headset 1 disconnected with error : "+err);
				if(fileOne!=null)fileOne.end();
				if(fileDrawOne!=null)fileDrawOne.end();
				fileDrawOneOpen=false;
				fileOneOpen=false;
				ClientOne = null;
		}else if(socket == ClientTwo ){
				console.log("headset 2 disconnected with error : "+err);
				if(fileTwo!=null)fileTwo.end();
				if(fileDrawTwo!=null)fileDrawOne.end();
				fileDrawTwoOpen=false;
				fileTwoOpen=false;
				ClientTwo =null;
		}else if(socket == ClientThree ){
				console.log("client 3 disconnected with error : "+err);
				ClientThree =null;
		}else{
			console.log(err);
		}
	})
});

/****************** END DRAW SERVER**************************/


/*function getIndexposition(tab,index){
	let i=0
	while(tab[i] != index){
		i++
	}
	return i;
}*/

function createDataFile(id){
	let file_name= new Date().toISOString().replace(/T/, '_').replace(/\..+/, '').replace(/:/,'-').replace(/:/,'-');
	console.log(file_name);
	if(id == 1){
		console.log("try create file One");
		file_name+="_headsetOne.txt";
	  	fileOne = fs.createWriteStream(path.join(DATA_PATH,file_name));
		fileOne.once("open", ()=>{
			fileOneOpen=true;
		});
	}else{
		file_name+="_headsetTwo.txt";
		fileTwo = fs.createWriteStream(path.join(DATA_PATH,file_name));
		fileTwo.once("open", ()=>{
			fileTwoOpen=true;
		});
	}
}

function createDrawFile(id){
	let file_name= new Date().toISOString().replace(/T/, '_').replace(/\..+/, '').replace(/:/,'-').replace(/:/,'-');
	console.log(file_name);
	if(id == 1){
		console.log("try create file One");
		file_name+="_headsetOne_draw.txt";
	  	fileDrawOne = fs.createWriteStream(path.join(DATA_PATH,file_name));
		fileDrawOne.once("open", ()=>{
			fileDrawOneOpen=true;
			ClientOne.write("\\senddraw;0;");
			if(SENDDRAW)ClientOne.write("\\affichedraw;0;");

		});
	}else{
		file_name+="_headsetTwo_draw.txt";
		fileDrawTwo = fs.createWriteStream(path.join(DATA_PATH,file_name));
		fileDrawTwo.once("open", ()=>{
			fileDrawTwoOpen=true;
			ClientTwo.write("\\senddraw;0;");
			if(SENDDRAW)ClientTwo.write("\\affichedraw;0;");
		});
	}
}

function getIndexposition(tab,index){
	let i=0
	while(tab[i] != index){
		i++
	}
	return i;
}

function gestionData(socket,data){
	let datas=data.toString().split(';');
	if(data.includes("\\position") && SAVEDATA){
			let index = getIndexposition(datas,"\\position");
			let saved = datas[index+2]+";"+datas[index+3]+";"+datas[index+4]+";\n";
			if(SAVEDATA)saveData(socket, saved);	
	}
	if(data.includes("\\startLine") && SAVEDRAW){
			let index = getIndexposition(datas,"\\startLine");
			let saved = "startLine;"+datas[index+2]+";"+datas[index+3]+";"+datas[index+4]+";\n";
			if(SAVEDRAW)saveDraw(socket, saved);
	}
	if(data.includes("\\newPoint") && SAVEDRAW){
			let index = getIndexposition(datas,"\\newPoint");
			let saved = "newPoint;"+datas[index+2]+";\n";
			if(SAVEDRAW)saveDraw(socket, saved);
	}
	if(data.includes("\\endLine") && SAVEDRAW){
			let saved = "endLine;\n";
			if(SAVEDRAW)saveDraw(socket, saved);
	}

}

function gestionPosition(socket,data){
	let datas=data.toString().split(';');
	if(data.includes("\\position")){
			let index = getIndexposition(datas,"\\position");
			

			let position="";

			if(socket == ClientOne){
				//Le casque n°1 à envoyé la donnée
				//position = "1;"+datas[index+3]+";"+datas[index+5]+";";
				position = "1 "+datas[index+3]+" "+(datas[index+5]=="True"?"1":"0")+" "+datas[index+2]+";";
			}else if (socket == ClientTwo){
				//Le casque n°2 à encoyé la donnée
				//position = "2;"+datas[index+3]+";"+datas[index+5]+";";
				position = "2 "+datas[index+3]+" "+(datas[index+5]=="True"?"1":"0")+" "+datas[index+2]+";";
			}

			// position = idCasque;positionMain;boolGachette;
			//TODO : Envoyé la variable position dans le flux de donnée
			if(ClientThree) ClientThree.write(position);

	}

}


function saveDraw(socket,data){
	if(socket == ClientOne && fileDrawOneOpen){
		fileDrawOne.write(data);
	}else if(fileDrawTwoOpen){
		fileDrawTwo.write(data);
	}
}

function saveData(socket, data){
	if(socket ==ClientOne && fileOneOpen){
		fileOne.write(data);
	}else if(fileTwoOpen){
		fileTwo.write(data);
	} 

}

function sendData(socket,data){
	if(socket == ClientOne){
		if(CLONE) ClientOne.write(data);
		else if(ClientTwo != null)ClientTwo.write(data)
	}else if (socket == ClientTwo){
		if(CLONE) ClientTwo.write(data);
		else if(ClientOne != null)ClientOne.write(data)
	}

}
