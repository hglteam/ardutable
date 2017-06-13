var totalColumns=1;
var totalRows=1;
var position=0;
var currentSteps=0;

var port;

var timer,sleepTime;

sleepTime = 6;

timer = createTimer();
timer.interval=sleepTime*10000;
timer.ontimeout=function() {stop();}
timer.start();


function sendCommand(command,data)
{
	if ((port) && (port.isOpen)) {		
		if (!data) data="";
		var rs = port.readAll();
		if (rs.length>0) {
			host.debugLog("Unread data: " + rs);
		}
		port.sendString(command.toString() + data.toString() + "\n");
		rv=port.readLine(100,"\r").toString().trim();
		timer.stop();
		timer.start();
		return rv;
	}
	timer.stop();
	timer.start();
	return false;
}


function exConnect() 
{
	host.messageLog("Conectando...");
	port=createSerialPort(cbSerial.currentValue,9600,"8n1");
	if (port.open()) {
		host.messageLog("Puerto abierto");
		port.sleep(500);
		port.sendString("dummy");
		port.sleep(500);
		port.sendString("V\n");
		port.sendString("V\n");
		var controller=port.readLine(100,"\r").toString().trim();
		host.messageLog("Controller Name:" + controller);
		if (controller!="") {
			control.setEnabled(true);
			host.connected(true);
			return true;
		} else {
			host.errorLog("No se encontró ninguna tornamesa.");
			port.close();
			control.setEnabled(false);
			host.connected(false);
			return false;
		}
	} else {
		host.debugLog("Puerto cerrado");
		control.setEnabled(false);
		host.connected(false);
		return false;
	}
}

function exDisconnect() 
{
	if (port) {
		port.close();
	}
	control.setEnabled(false);
	return true;
}

function exIsConnected()
{
	return port.isOpen;
}


function getPosition()
{
	var position = sendCommand("C","0");
	if (position===false) {
		return false;
	} else {
		return Number(position);	
	}
}

function resetPosition()
{
	var reset = sendCommand("C","1");
		return reset;	
}

function moveToZero()
{
	var zero = sendCommand("C","2");
		return zero;	
}

function stop() 
{
	sendCommand("C","4");
	control.setEnabled(false);
	timer.stop();
}

function unstop() 
{
	sendCommand("C","5");
	control.setEnabled(true);
	timer.start();
}


function status() 
{
	var status = sendCommand("C","3");
	return Number(status);
}

var abort=false;

function exAbort() 
{
	stop();
	abort=true;
}

function exStep() 
{
	
	var step = 360/totalColumns;
	var available = false;
	sendCommand("M30",step.toString());
	do{
	available = status();
	}
	while(!available);
	currentSteps++;
	if(currentSteps===totalColumns){
	currentSteps=0;
	moveToZero();
	}
	return true;
}

function exMoveTo(c,r) 
{
	var position = getPosition();
	var distanceFromZero = c*360/totalColumns;
	var distance = position - distanceFromZero;
	if (distance < 0){
		sendCommand("M31",-distance);
	}
	else if (distance > 0){
		sendCommand("M30",distance);
	}
	port.sleep(1000)
	return true;
}

function exStart(cols,rows)
{
	totalColumns=cols;
	totalRows=rows;
	return true;
}

function detectPort() {
	var i=0;
	while(i<cbSerial.count()) {
		cbSerial.setCurrentIndex(i);
		host.processEvents();
		if (exConnect()) return true;
		i++;
	}
	return false;
}


var cbSerial,sbSteps;

setup.addLabel("Puerto: ");
cbSerial=setup.addPortCombobox();
cbSerial.currentValue=host.loadSetting("port_obji");

bt=setup.addButton("Conectar");
bt.onclick=exConnect;
if (host.language()=="en") {
	bt=setup.addButton("Automatic detection");
} else {
	bt=setup.addButton("Detección automática");
}
bt.onclick=function() { detectPort(); }
setup.addNewLine();
bt=setup.addButton("Desbloquear");
bt.onclick=function() { unstop(); };

//setup.addLabel("Tiempo de desactivación:");
//sbSteps=setup.addSpinBox(0,120);
//sbSteps.value=sleepTime;
//sbSteps.onchange=function() { sleepTime=sbSteps.value; }

v=control.addButton("<");
v.onclick=function() { sendCommand("M","2");};
v=control.addButton("Bloquear");
v.onclick=function() { stop(); };
v=control.addButton(">");
v.onclick=function() { sendCommand("M","1");};
v=control.addButton("Posición actual");
v.onclick=function() { host.messageLog(getPosition()); };
v=control.addButton("Redefinir punto cero");
v.onclick=function() { host.messageLog(resetPosition()); };
v=control.addButton("Mover al punto cero");
v.onclick=function() { host.messageLog(moveToZero()); };
v=control.addButton("Estado");
v.onclick=function() { host.messageLog(status()); };

control.setEnabled(false);