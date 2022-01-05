// rf69 demo tx rx.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF69 class. RH_RF69 class does not provide for addressing or
// reliability, so you should only use RH_RF69  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf69_server.
// Demonstrates the use of AES encryption, setting the frequency and modem 
// configuration


#include <SPI.h>
#include <RH_RF69.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include <Adafruit_SleepyDog.h>

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 433.0

#if defined (__AVR_ATmega32U4__) // Feather 32u4 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     7
  #define RFM69_RST     4
  #define LED           13
#endif

#if defined(ADAFRUIT_FEATHER_M0) // Feather M0 w/Radio
  #define RFM69_CS      8
  #define RFM69_INT     3
  #define RFM69_RST     4
  #define LED           13
#endif

#if defined (__AVR_ATmega328P__)  // Feather 328P w/wing
  #define RFM69_INT     3  // 
  #define RFM69_CS      4  //
  #define RFM69_RST     2  // "A"
  #define LED           13
#endif

#if defined(ESP8266)    // ESP8266 feather w/wing
  #define RFM69_CS      2    // "E"
  #define RFM69_IRQ     15   // "B"
  #define RFM69_RST     16   // "D"
  #define LED           0
#endif

#if defined(ESP32)    // ESP32 feather w/wing
  #define RFM69_RST     13   // same as LED
  #define RFM69_CS      33   // "B"
  #define RFM69_INT     27   // "A"
  #define LED           13
#endif

/* Teensy 3.x w/wing
#define RFM69_RST     9   // "A"
#define RFM69_CS      10   // "B"
#define RFM69_IRQ     4    // "C"
#define RFM69_IRQN    digitalPinToInterrupt(RFM69_IRQ )
*/
 
/* WICED Feather w/wing 
#define RFM69_RST     PA4     // "A"
#define RFM69_CS      PB4     // "B"
#define RFM69_IRQ     PA15    // "C"
#define RFM69_IRQN    RFM69_IRQ
*/

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// Singleton BNO055 instance
Adafruit_BNO055 bno = Adafruit_BNO055(55);

#define VBATPIN A9

uint8_t inBuf[RH_RF69_MAX_MESSAGE_LEN];
uint8_t len;
unsigned long startTime;

char outBuf[40] = "222222222";
int16_t packetnum = 0;  // packet counter, we increment per xmission

unsigned char address = 1;
#define EE_AD 0

void setAddress(unsigned char a)
{
  if(a >= 1 && a <= 100) address = a;
  outBuf[0] = address + '0';
}

void setup() 
{
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);
  
  if (!rf69.init()) {
    Serial.println("RFM69 radio init failed");
    while (1);
  }
  Serial.println("RFM69 radio init OK!");
  
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println("setFrequency failed");
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  /* Initialise the BNO055 sensor */
  delay(200); 
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.println("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
  }  
  delay(100); 
  bno.setExtCrystalUse(true);
  
  pinMode(LED, OUTPUT);
  
  Serial.print("RFM69 radio @");  Serial.print((int)RF69_FREQ);  Serial.println(" MHz");
  setAddress(EEPROM.read(EE_AD));
}


/*int16_t vBat;
int16_t accel[3];
int16_t quat[4];*/
#define vBat *((int16_t*)(outBuf + 1) + 0)
#define accel ((int16_t*)(outBuf + 1) + 1)
#define quat ((int16_t*)(outBuf + 1) + 4)

void aquire() {
  uint8_t buffer[8];
  vBat = analogRead(VBATPIN);
  bno.readLen(bno.BNO055_LINEAR_ACCEL_DATA_X_LSB_ADDR, (char*)accel, 6);
  bno.readLen(bno.BNO055_QUATERNION_DATA_W_LSB_ADDR, (char*)quat, 8);
}

char command[32];
unsigned char commandLen = 0;

void readCommand()
{
  char c;
  if(Serial.available()) {
    c = Serial.read();
    if(c == 10) {
      if(commandLen >= 2 && command[0] == 'A') {
        setAddress(command[1] - '0');
        EEPROM.write(EE_AD, address);
        Serial.print("New address: ");
        Serial.println(address);
      }
      commandLen = 0;
    } else {
      if(commandLen < sizeof(command))
        command[commandLen++] = c; 
    }
  }   
}
void loop() {
  aquire();
  readCommand();
  if (rf69.available()) {
    // Should be a message for us now   
    len = sizeof(inBuf);
    if (rf69.recv(inBuf, &len)) {
      if (!len) return;
      if(inBuf[0] != '0') return;
      //Serial.print("RSSI: ");
      //Serial.println(rf69.lastRssi(), DEC);

      // Wait the allowed time slice
      delay((address - 1)* 5);
      
      // Send a reply!
      rf69.send(outBuf, 17);
      digitalWrite(LED, HIGH);
      startTime = millis();
      rf69.waitPacketSent();
    } else {
      Serial.println("Receive failed");
    }
  } else {
    if(millis() - startTime > 4000) {
      digitalWrite(LED, LOW);
      rf69.sleep();
      bno.enterSuspendMode();
      Watchdog.sleep(4000);

      bno.enterNormalMode();
      digitalWrite(LED, HIGH);
      startTime = millis() - 3900;
    }
  }
}


void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i=0; i<loops; i++)  {
    digitalWrite(PIN,HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN,LOW);
    delay(DELAY_MS);
  }
}
