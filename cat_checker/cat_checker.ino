/*
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 * 
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 * 
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * Example sketch/program showing how to read data from a PICC (that is: a RFID
 * Tag or Card) using a MFRC522 based RFID Reader on the Arduino SPI interface.
 * 
 * When the Arduino and the MFRC522 module are connected (see the pin layout
 * below), load this sketch into Arduino IDE then verify/compile and upload it.
 * To see the output: use Tools, Serial Monitor of the IDE (hit Ctrl+Shft+M).
 * When you present a PICC (that is: a RFID Tag or Card) at reading distance
 * of the MFRC522 Reader/PCD, the serial output will show the ID/UID, type and
 * any data blocks it can read. Note: you may see "Timeout in communication"
 * messages when removing the PICC from reading distance too early.
 * 
 * If your reader supports it, this sketch/program will read all the PICCs
 * presented (that is: multiple tag reading). So if you stack two or more
 * PICCs on top of each other and present them to the reader, it will first
 * output all details of the first and then the next PICC. Note that this
 * may take some time as all data blocks are dumped, so keep the PICCs at
 * reading distance until complete.
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 */

#include <SPI.h>
#include <MFRC522.h>
#include "Timer.h"

#define RST_PIN		9		// 
#define SS_PIN		10		//

#define CAT_0 0
#define CAT_1 1

#define CAT_ID0 " 196 204 250 53"
#define CAT_ID1 " 50 123 75 43"

MFRC522 mfrc522(SS_PIN, RST_PIN);	// Create MFRC522 instance"
Timer t;



int ledPinsCat[2][3] = {{2,3,4}, {5,6,7}};

const int NUMBER_OF_PINS = sizeof(ledPinsCat[0])/sizeof(ledPinsCat[0][0]);
const int NUMBER_OF_LEDS = NUMBER_OF_PINS * (NUMBER_OF_PINS - 1);

byte ledPairs[NUMBER_OF_LEDS/2][2] = {{2,1}, {1,0}, {2,0}};

int MAX_TIME = 10000;
int TIMER_TICK = 1000;

int CAT_TIME[2] = {MAX_TIME ,MAX_TIME};
int tickEvent = 0;


void setup() {
	Serial.begin(9600);		// Initialize serial communications with the PC
	while (!Serial);		// Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();		// Init MFRC522

	Serial.println(F("Scan PICC to see UID, type, and data blocks..."));

        tickEvent = t.every(TIMER_TICK, doSomething);
        resetTimer(CAT_1);
        resetTimer(CAT_0);
}

void loop() {
   
        t.update();
        
	// Look for new cards
	if ( ! mfrc522.PICC_IsNewCardPresent()) {
		return;
	}

	// Select one of the cards
	if ( ! mfrc522.PICC_ReadCardSerial()) {
		return;
	}

        String scannedTagID = getTagID();
        resetTimer(getCatFromTagID(scannedTagID));
        
        Serial.print("Read tag ID: "); Serial.println(scannedTagID);  
             
}



int getCatFromTagID(String tagID){
  if(tagID.equals(CAT_ID0)){
    return CAT_0;
  }
  else if (tagID.equals(CAT_ID1)){
    return CAT_1;
  }
  else
    return -1;
}


String getTagID(){
 //Serial.print(F("Card UID:"));
 String tagID = "";
        for (byte i = 0; i < mfrc522.uid.size; i++) {
                tagID = tagID + (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                tagID = tagID + (mfrc522.uid.uidByte[i]);
        }
  return tagID;
}


int timerTick(int currCat){
  return CAT_TIME[currCat] -= TIMER_TICK;
}


void resetTimer(int currCat){
  if(CAT_TIME[CAT_0] < 0 && CAT_TIME[CAT_1] < 0)
    tickEvent = t.every(TIMER_TICK, doSomething);
  CAT_TIME[currCat] = MAX_TIME;  
  
}


void updateLedPinsCat(int currCat){
  
  int led = (CAT_TIME[currCat] / TIMER_TICK) * 6 / (MAX_TIME / TIMER_TICK);
  Serial.println(led);
  //lightLed(CAT_TIME[currCat]/TIMER_TICK, ledPinsCat[currCat]);
  lightLed(led, ledPinsCat[currCat]);

}


void doSomething(){
  if(timerTick(CAT_0) >= 0){
    updateLedPinsCat(CAT_0);
  }
  if(timerTick(CAT_1) >= 0){
    updateLedPinsCat(CAT_1); 
  }  
  
  if(CAT_TIME[CAT_0] < 0 && CAT_TIME[CAT_1] < 0)
    t.stop(tickEvent);
}


void lightLed(int led, int ledPins[]) {

  int indexA = ledPairs[led/2][0];
  int indexB = ledPairs[led/2][1];
  int ledPinA = ledPins[indexA];
  int ledPinB = ledPins[indexB];

  // turn off all LEDs not connected to given LED
  for(int i = 0; i < NUMBER_OF_PINS; i++) {
    if (ledPins[i] != ledPinA && ledPins[i] != ledPinB) {
      // set pinmode to input
      pinMode(ledPins[i], INPUT);
      // turn off pull-up resistor
      digitalWrite(ledPins[i], LOW);
    }
  }

  pinMode(ledPinA, OUTPUT);
  pinMode(ledPinB, OUTPUT);

  if (led % 2 == 0) {
    digitalWrite(ledPinA, LOW);
    digitalWrite(ledPinB, HIGH);
  } else {
    digitalWrite(ledPinA, HIGH);
    digitalWrite(ledPinB, LOW);
  }
}




