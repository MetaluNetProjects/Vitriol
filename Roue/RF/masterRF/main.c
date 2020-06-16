/*********************************************************************
 *               RF communication master
 *                for: La Roue / Vitriol
 *                metalu.net 2020
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <RF24.h>
#include "../protocol.h"

t_delay mainDelay;
//uint8_t rfconnected = 0;
uint8_t addresses[6][6];

uint8_t txpipe[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
uint8_t rxpipe[5] = {0xC2, 0xC2, 0xC2, 0xC2, 0xC2};
//uint8_t rcvID = 0;
byte oldRFservice = 0;
byte RFpoll = 0;
byte testVar = 11;

static void delay(unsigned long millisecs)
{
	t_delay del;
	delayStart(del, millisecs * 1000);
	while(!delayFinished(del)){}
}

void setupPipes()
{
	uint8_t i;

	// setup pipes
	RF24_stopListening();
	/*for(i = 0; i < 6 ; i++) {
		fillPipeAddress(addresses[i], i + 1, ID_MASTER);
		RF24_openReadingPipe(i, addresses[i]);
	}*/
	fillPipeAddress(addresses[0], ID_CAMERA, ID_MASTER);
	RF24_openReadingPipe(1, addresses[0]);

	RF24_startListening();
}

void setupRXPipe()
{
	//RF24_stopListening();
	//fillPipeAddress(rxpipe, ID_MASTER, ID_MASTER);
	//RF24_openReadingPipe(1, rxpipe);

	RF24_startListeningFast();
}


void initRF()
{
	printf("C initRF\n");
	rfconnected = 0;
	RF24_init();
	delay(100);
	if(!RF24_isChipConnected()) return;
	rfconnected = 1;
	//return;
	RF24_disableDynamicPayloads();
	RF24_enableDynamicPayloads();
	RF24_enableDynamicAck();
	RF24_enableAckPayload();
	RF24_setAutoAck(0);

	//RF24_setRetries(8, 3);
	if(oldRFservice) setupPipes();
	//else setupRXPipe();
	fillPipeAddress(rxpipe, ID_MASTER, ID_MASTER);
	RF24_openReadingPipe(1, rxpipe);
}

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED);		// set the LED pin mode to digital out
	digitalClear(LED);			// clear the LED

	//oldRFservice = 1;
	pinModeDigitalOut(RF24_POWER);
	digitalClear(RF24_POWER);
	
	delay(100);
	
	initRF();
	//setupPipes();
	delayStart(mainDelay, 5000);// init the mainDelay to 5 ms
}

int loops = 0;

uint8_t rfService(); // return ACTIVITY
uint8_t rfComm();

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	
	//if(oldRFservice) {
		if(rfService()) digitalSet(LED);
		else digitalClear(LED);
	//}
	
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 6000); 	// re-init mainDelay
		/*if(rfComm()) digitalSet(LED);
		else {
			digitalClear(LED);
		}*/
		if(rfconnected && (RFpoll != 0)) rfComm();
		if(loops++ == 50) {
			loops = 0;
			printf("CR C %d %d\n", rfconnected, RF24_available());
			if(!RF24_isChipConnected()) rfconnected = 0;
			if(!rfconnected) initRF();
		}
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c = fraiseGetChar();
	if(c == 'L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	}
	else if(c == 'R'){		//reset radio
		initRF();
	}
	else if(c == 'C') {
		if(rfComm()) digitalSet(LED);
		else digitalClear(LED);
	}
	else if(c == 'O') {
		c = fraiseGetChar();
		oldRFservice = (c != '0');
		if(c != '0') setupPipes();
		else {
			RF24_stopListening();
			//setupRXPipe();
		}
	}
	else if(c == 'P') {
		c = fraiseGetChar();
		if(c != '0') RF24_powerUp();
		else RF24_powerDown();
	}
	else if(c == 'r') {
		c = fraiseGetChar() -'0';
		switch(c) {
			case 0: RF24_closeReadingPipe(1); break;
			case 1: RF24_setAddressWidth(5); break;
			case 2: RF24_setChannel(76); break;
			case 3: RF24_toggle_features(); break;
			case 4: if(!RF24_setDataRate(RF24_1MBPS)) printf("C err\n"); break;
			case 5:   // Flush buffers
					  RF24_flush_rx();
					  RF24_flush_tx();
					  break;
			case 6: break;
		}
	}
	else if(c == 'p') {
		c = fraiseGetChar();
		if(c == '0') digitalSet(RF24_POWER);
		else digitalClear(RF24_POWER);
		/*if(c != '0') pinModeDigitalOut(RF24_POWER);
		else pinModeDigitalIn(RF24_POWER);*/
	}
	else if(c == 'M') {
		byte regs[32] = {'B', 31};
		byte i;
		for (i = 0; i < 29 ; i++) regs[i+2] = RF24_read_register(i);
		fraiseSend(regs, 32);
	}
	else if(c == 'A') {
		byte regs[17] = {'B', 32};
		byte i;
		RF24_read_pipes_addresses(regs+2);
		fraiseSend(regs, 17);
	}
}

void fraiseReceive() // receive raw
{
	unsigned char c;
	
	c = fraiseGetChar();
	if(c == 1) {
		testVar = fraiseGetChar();
		if(rfComm()) digitalSet(LED);
		else digitalClear(LED);
	}
	else if(c == 2) {
		RFpoll = fraiseGetChar();
	}
}

uint8_t pipe_num, rcvLen, rcvBuffer[36] = {'B', 30}, sndBuffer[32];

#define RF_INACTIVE_TIME 5000000UL
t_time t1, t2, t3;
t_time startTx;

uint8_t rfService()
{
	uint8_t size;
	
	static t_delay rfDelay = 0;
	static uint8_t active = 0;
	
	if(delayFinished(rfDelay)) active = 0;
	
	if(!RF24_pipeAvailable(&pipe_num)) return active;
	size = RF24_getDynamicPayloadSize();
	if(size < 1) return active; // Corrupt payload has been flushed
	
	//rcvBuffer[2] = pipe_num;
	RF24_read(rcvBuffer + 2, 32);
	rcvBuffer[size + 2] = '\n';
	fraiseSend(rcvBuffer, size + 3);
	
	//printf("CT %ld %ld %ld %ld\n", timeToMicro(t1), timeToMicro(t2), timeToMicro(t3), timeToMicro(elapsed(startTx)));
	
	active = 1;
	delayStart(rfDelay, RF_INACTIVE_TIME);
	return active;
}

uint8_t rfComm()
{
	static uint8_t size;
	static uint8_t count = 0, destID = ID_AXL_A;
	static t_delay rfDelay = 0;
	static uint8_t active = 0;
	
	//active = 1;
	if(delayFinished(rfDelay)) active = 0;
	
	switch(count) {
		case 0 : destID = ID_CAMERA; break;
		case 1 : destID = ID_AXL_A; break;
		case 2 : destID = ID_AXL_B; break;
		case 3 : destID = ID_AXL_C; break;
		case 4 : destID = ID_AXL_D; break;
		case 5 : destID = ID_AXL_E; break;
	}
	count++;
	if(count > RFpoll) count = 0;
	
	startTx = time();
	//RF24_stopListening(); // 2ms!
	RF24_stopListeningFast(); // 1ms
	t1 = elapsed(startTx);
	fillPipeAddress(txpipe, ID_MASTER, destID);
	RF24_openWritingPipe(txpipe); // 1ms
	//
	
	// 1.5ms:
	sndBuffer[0] = testVar;
	sndBuffer[1] = 100;
	sndBuffer[2] = 150;
	sndBuffer[3] = 200;
	if(!RF24_write(sndBuffer, 4, 1)) {
		byte tx_ok, tx_fail, rx_ready;
		RF24_whatHappened(&tx_ok, &tx_fail, &rx_ready);
		printf("CR err %d %d %d\n", tx_ok, tx_fail, rx_ready);
		RF24_txStandBy();
		return active;
	}
	//
	RF24_txStandBy(); // 0.4ms
	t2 = elapsed(startTx);
	//setupRXPipe(); // 3.3ms ; 2.3ms with startListeningFast
	RF24_startListeningFast(); // 1ms
	t3 = elapsed(startTx);

	active = 1;
	delayStart(rfDelay, RF_INACTIVE_TIME);

	return active;


// I couldn't make the ack payload feature to work!
#if 0

	if(!RF24_available()) return active;
	//if(!RF24_isAckPayloadAvailable) return active;
	size = RF24_getDynamicPayloadSize();
	//size = 6;

	if(size < 1) return active; // Corrupt payload has been flushed
	
	//rcvBuffer[2] = destID;
	RF24_read(rcvBuffer + 2, 32);
	rcvBuffer[size + 2] = '\n';
	fraiseSend(rcvBuffer, size + 3);
	
	/*if(rcvBuffer[2] == CMD_MOTOR) {
		((char*)&PWM)[1] = rcvBuffer[3];
		((char*)&PWM)[0] = rcvBuffer[4];
		//pwm /= 4;
		printf("C M %d\n", PWM);
	} else if(rcvBuffer[2] == CMD_LAMP) {
		if(rcvBuffer[3] == 0) {
			((char*)&lampTarget1)[1] = rcvBuffer[4];
			((char*)&lampTarget1)[0] = rcvBuffer[5];
			printf("C LT1 %d\n", lampTarget1);
		} else if(rcvBuffer[3] == 1) {
			((char*)&lampTarget2)[1] = rcvBuffer[4];
			((char*)&lampTarget2)[0] = rcvBuffer[5];
			printf("C LT2 %d\n", lampTarget2);
		}
	}
	else if(rcvBuffer[2] == CMD_LAMP_SPEED) {
		lampSpeed = rcvBuffer[3];
	}
	else if(rcvBuffer[2] == CMD_MOTOR_LAMPS) {
		((char*)&PWM)[1] = rcvBuffer[3];
		((char*)&PWM)[0] = rcvBuffer[4];
		((char*)&lampTarget1)[1] = rcvBuffer[5];
		((char*)&lampTarget1)[0] = rcvBuffer[6];
		((char*)&lampTarget2)[1] = rcvBuffer[7];
		((char*)&lampTarget2)[0] = rcvBuffer[8];
	}*/
	
	return active;
#endif
}

