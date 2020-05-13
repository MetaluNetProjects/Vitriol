/*********************************************************************
 *               SEN0158 example for Versa1.0
 *	SCL is K5 and SDA is K7.
 *********************************************************************/

#define BOARD Versa1

#include <fruit.h>
#include <SEN0158.h>
#include <i2c_master.h>

t_delay mainDelay;
SEN0158 cam1;

void setup(void) {	
//----------- Setup ----------------
	fruitInit();
			
	pinModeDigitalOut(LED);		// set the LED pin mode to digital out
	digitalClear(LED);			// clear the LED
	delayStart(mainDelay, 5000);// init the mainDelay to 5 ms

	// setup I2C master:
	i2cm_init(I2C_MASTER, I2C_SLEW_ON, FOSC/400000/4-1);
	
	// setup SEN0158:
	SEN0158Init(&cam1);
}

byte camRun = 0;

void loop() {
// ---------- Main loop ------------
	fraiseService();	// listen to Fraise events
	if(camRun) SEN0158Service(&cam1);

	if(delayFinished(mainDelay)) // when mainDelay triggers :
	{
		delayStart(mainDelay, 10000); 	// re-init mainDelay
		if(camRun) SEN0158Send(&cam1, 1);
	}
}

// Receiving

void fraiseReceiveChar() // receive text
{
	unsigned char c;
	
	c=fraiseGetChar();
	if(c=='L'){		//switch LED on/off 
		c=fraiseGetChar();
		digitalWrite(LED, c!='0');		
	} else if(c=='R') {
		// reinit SEN0158:
		c=fraiseGetChar();
		if(c == '0') camRun = 0;
		else {
			SEN0158Init(&cam1);
			camRun = 1;
		}
	}
}

void fraiseReceive()
{
	byte c = fraiseGetChar();
	if(c == 100) {
		SEN01585SetDistanceThreshold(fraiseGetInt());
	}
}
