/*********************************************************************
 *               RF communication master
 *                for: La Roue / Vitriol
 *                metalu.net 2020
 *********************************************************************/

#define BOARD Versa2

#include <fruit.h>
#include <analog.h>
#include <RF24.h>
#include <ADXL345.h>
#include <i2c_master.h>

#include "../protocol.h"

t_delay mainDelay;
uint8_t rfconnected = 0;
//uint8_t txpipe[5];
uint8_t rxpipe[5];
uint8_t rfID = ID_AXL_A;
//uint8_t RFRXbuffer[32];
uint8_t RFTXbuffer[32];

ADXL345 adxl1;

void fillRFTX();

void setupPipes()
{
	// setup pipes
	RF24_stopListening();
	//fillPipeAddress(txpipe, rfID, ID_MASTER);
	fillPipeAddress(rxpipe, ID_MASTER, rfID);
	RF24_openReadingPipe(0, rxpipe);
	RF24_startListening();
	//fillRFTX();
}

void initRF()
{
	printf("C initRF\n");
	rfconnected = 0;
	RF24_init();
	if(!RF24_isChipConnected()) return;
	rfconnected = 1;
	RF24_enableDynamicPayloads();
	//RF24_enableDynamicAck();
	//RF24_enableAckPayload();
	//RF24_setRetries(8, 3);
	RF24_setAutoAck(0);

	setupPipes();
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
	
	// setup ADXL345:
	ADXL345Init(&adxl1, 0); // 1st ADXL345's SDO pin is high voltage level

	initRF();
}

int loops = 0;

uint8_t rfService(); // return ACTIVITY

/*void sendRF()
{
	//static char turn = 0;
	static int count = 0;
	static int v_batt = 0;

	RF24_openWritingPipe(txpipe);
	if(count%256 == 0) {
		//turn = 1;
		v_batt = analogGet(0);
		RFRXbuffer[0] = rfID;
		RFRXbuffer[1] = CMD_VBATT;
		RFRXbuffer[2] = v_batt >> 8;
		RFRXbuffer[3] = v_batt & 255;
		if(RF24_write(RFRXbuffer, 4, 0)) {};
	}
	else {
		//turn = 0;
		RFRXbuffer[0] = rfID;
		RFRXbuffer[1] = CMD_AXL;
		RFRXbuffer[2] = adxl1.xl;
		RFRXbuffer[3] = adxl1.xh;
		RFRXbuffer[4] = adxl1.yl;
		RFRXbuffer[5] = adxl1.yh;
		RFRXbuffer[6] = adxl1.zl;
		RFRXbuffer[7] = adxl1.zh;
		if(RF24_write(RFRXbuffer, 8, 0)) {};
	}
	count++;
	RF24_txStandBy();
}*/

void fillRFTX()
{
	static int v_batt = 0;
	static byte l;

	l = 0;
	v_batt = analogGet(0);
	RFTXbuffer[l++] = rfID;
	RFTXbuffer[l++] = CMD_AXL;
	RFTXbuffer[l++] = v_batt >> 8;
	RFTXbuffer[l++] = v_batt & 255;
	RFTXbuffer[l++] = adxl1.xl;
	RFTXbuffer[l++] = adxl1.xh;
	RFTXbuffer[l++] = adxl1.yl;
	RFTXbuffer[l++] = adxl1.yh;
	RFTXbuffer[l++] = adxl1.zl;
	RFTXbuffer[l++] = adxl1.zh;
	RF24_writeAckPayload(0, RFTXbuffer, l);
}

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	analogService();
	ADXL345Service(&adxl1);

	if(rfService()) digitalSet(LED);
	else {
		digitalClear(LED);
	}
	
	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 5000 /*+ rfID*99*/); 	// re-init mainDelay
		//if(rfconnected) sendRF();
		//if(rfconnected) fillRFTX();

		if(loops++ == 100) {
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
}

void fraiseReceive() // receive raw
{
	unsigned char c;
	
	c = fraiseGetChar();
}

uint8_t /*pipe_num, rcvLen,*/ rcvBuffer[36] = {'B', 30};

#define RF_INACTIVE_TIME 5000000UL

uint8_t rfService()
{
	uint8_t size;
	
	static t_delay rfDelay = 0;
	static uint8_t active = 0;
	
	if(delayFinished(rfDelay)) active = 0;
	
	if(!RF24_available()) return active;
	size = RF24_getDynamicPayloadSize();
	if(size < 1) return active; // Corrupt payload has been flushed
	
	RF24_read(rcvBuffer + 2, 32);
	rcvBuffer[size + 3] = '\n';
	fraiseSend(rcvBuffer, size + 3);
	//fillRFTX();
	
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
//#endif
