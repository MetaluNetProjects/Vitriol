/*********************************************************************
 *               RF communication master
 *                for: La Roue / Vitriol
 *                metalu.net 2020
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <analog.h>
#include <RF24.h>
#include <i2c_master.h>
#include <SEN0158.h>
#include "../protocol.h"

t_delay mainDelay;
uint8_t rfID = ID_CAMERA;
uint8_t RFTXbuffer[32];
SEN0158 cam1;

/*void setupPipes()
{
	// setup pipes
	//RF24_stopListening();
	fillPipeAddress(txpipe, rfID, ID_MASTER);
	//RF24_startListening();
}*/

void initRF()
{
	printf("C initRF\n");
/*	rfconnected = 0;
	RF24_init();
	if(!RF24_isChipConnected()) return;
	rfconnected = 1;
	RF24_enableDynamicPayloads();
	RF24_setRetries(0, 2);
	setupPipes();*/
	initSlaveRF(rfID);
}

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED);		// set the LED pin mode to digital out
	digitalClear(LED);			// clear the LED
	delayStart(mainDelay, 5000);// init the mainDelay to 5 ms

//----------- Analog setup ----------------
	analogInit();			// init analog module
	VREFCON0bits.FVREN = 1;
	VREFCON0bits.FVRS = 2; 	// vref = 2.048V
	ADCON1bits.PVCFG = 2; 	// ADC positive ref = FVR BUF2
	analogSelect(0, VBATT);	// assign VBatt to analog channel 0

	// setup I2C master:
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
	
	// setup SEN0158:
	SEN0158Init(&cam1);

	initRF();
}

int loops = 0;
byte camRun = 1;
uint8_t rfService(); // return ACTIVITY

void sendRF()
{
	//static char turn = 0;
	static int count = 0;
	static int v_batt = 0;
	byte l, i;

	l = 0;

	RF24_stopListeningFast();
	//RF24_openWritingPipe(txpipe);
	RFTXbuffer[l++] = rfID;

	if(count%16 == 0) {
		//turn = 1;
		v_batt = analogGet(0);
		RFTXbuffer[l++] = CMD_VBATT;
		RFTXbuffer[l++] = v_batt >> 8;
		RFTXbuffer[l++] = v_batt & 255;
	}
	else {
		//turn = 0;
		RFTXbuffer[l++] = CMD_CAMERA;
		for (i = 0; i < SEN0158_NB_WINNERS; i++) {
			RFTXbuffer[l++] = cam1.points[cam1.winners[i]].x & 0xFF;
			RFTXbuffer[l++] = cam1.points[cam1.winners[i]].x >> 8;
			RFTXbuffer[l++] = cam1.points[cam1.winners[i]].y & 0xFF;
			RFTXbuffer[l++] = cam1.points[cam1.winners[i]].y >> 8;
			RFTXbuffer[l++] = cam1.points[cam1.winners[i]].score;
		}
	}
	count++;
	RF24_write(RFTXbuffer, l, 1);
	RF24_txStandBy();
	RF24_startListeningFast();
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();
	if(camRun) SEN0158Service(&cam1);

	if(rfService()) digitalSet(LED);
	else {
		digitalClear(LED);
	}
	
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 10000); 	// re-init mainDelay
		if(camRun) SEN0158Send(&cam1, 1);
		if(loops++ == 50) {
			loops = 0;
			printf("CR C %d\n", rfconnected);
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
	else if(c=='C') {
		// reinit SEN0158:
		c=fraiseGetChar();
		if(c == '0') camRun = 0;
		else {
			SEN0158Init(&cam1);
			camRun = 1;
		}
	}
}

void fraiseReceive() // receive raw
{
	unsigned char c;
	
	c = fraiseGetChar();
	if(c == 100) {
		SEN01585SetDistanceThreshold(fraiseGetInt());
	} else if(c == 101) {
		SEN0158SetupFromFraise(&cam1);
	} 
}

#if 1
uint8_t /*pipe_num, rcvLen,*/ rcvBuffer[36] = {'B', 30};

#define RF_INACTIVE_TIME 5000000UL

uint8_t rfService()
{
	uint8_t size;
	
	static t_delay rfDelay = 0;
	static uint8_t active = 0;
	
	if(delayFinished(rfDelay)) active = 0;
	
	//if(!RF24_pipeAvailable(&pipe_num)) return active;
	if(!RF24_available()) return active;
	size = RF24_getDynamicPayloadSize();
	if(size < 1) return active; // Corrupt payload has been flushed
	
	RF24_read(rcvBuffer + 2, 32);
	rcvBuffer[size + 3] = '\n';
	fraiseSend(rcvBuffer, size + 3);
	
	if(rcvBuffer[2] == 11) sendRF();
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
	
	active = 1;
	delayStart(rfDelay, RF_INACTIVE_TIME);
	return active;
}
#endif
