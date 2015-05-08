/*
 * -----------------------------------------------------------------------------------------
 * The cat_checker controls 12 leds, divided in two groups of 6 associated with a NFC tag: 
 * each led group represents an abstract amount of time since the last time the cat was 
 * seen. The time is reset everytime the rfid reads a tag.
 * 
 * Typical pin layout used for the RFID module:
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

// Define pin numbers
#define RST_PIN		9		 
#define SS_PIN		10		

// Define cats unique id (used to access arrays)
#define CAT_0 0
#define CAT_1 1

// Define NFC tags id
#define CAT_ID0 " 196 204 250 53"
#define CAT_ID1 " 50 123 75 43"

// Create MFRC522 instance
MFRC522 mfrc522(SS_PIN, RST_PIN);	
Timer t;

// Define variables to control pins with charlieplexing
int ledPinsCat[2][3] = {{2,3,4}, {5,6,7}};
const int NUMBER_OF_PINS = sizeof(ledPinsCat[0])/sizeof(ledPinsCat[0][0]);
const int NUMBER_OF_LEDS = NUMBER_OF_PINS * (NUMBER_OF_PINS - 1);

byte ledPairs[NUMBER_OF_LEDS/2][2] = {{2,1}, {1,0}, {2,0}};

// Define variables to control the maximum amount of time allowed to a cat
// to show up before the last led becomes red
int MAX_TIME = 10000;
int TIMER_TICK = 1000;
int tickEvent = 0;

int CAT_TIME[2] = {MAX_TIME ,MAX_TIME};


void setup() {
        // Initialize serial communications with the PC
	Serial.begin(9600);	
        // Do nothing if no serial port is opened	
	while (!Serial);
        // Init SPI bus		
	SPI.begin();	
        // Init MFRC522		
	mfrc522.PCD_Init();		

	Serial.println(F("Scan PICC to see UID, type, and data blocks..."));

        tickEvent = t.every(TIMER_TICK, doRecurrentTask);
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


// Get the corresponding unique cat id for the given NFC tag id
int getCatFromTagID(String tagID){
        if(tagID.equals(CAT_ID0))
                return CAT_0;
        
        else if (tagID.equals(CAT_ID1))
                return CAT_1;       
        else
                return -1;
}


// Get the NFC tag id reading from the RFID module
String getTagID(){
       String tagID = "";
       for (byte i = 0; i < mfrc522.uid.size; i++) {
                tagID = tagID + (mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
                tagID = tagID + (mfrc522.uid.uidByte[i]);
        }
        return tagID;
}


// For each tick of the timer decrement the value for the 
// currCat fo a TIMER_TICK 
int timerTick(int currCat){
        return CAT_TIME[currCat] -= TIMER_TICK;
}

// Reset the CAT_TIME value for the currCat
void resetTimer(int currCat){
        if(CAT_TIME[CAT_0] < 0 && CAT_TIME[CAT_1] < 0)
                tickEvent = t.every(TIMER_TICK, doRecurrentTask);
        CAT_TIME[currCat] = MAX_TIME;  
  
}


// Update led according to the CAT_TIME value for the currCat
void updateLedPinsCat(int currCat){
        int led = (CAT_TIME[currCat] / TIMER_TICK) * 6 / (MAX_TIME / TIMER_TICK);
        Serial.println(led);
        //lightLed(CAT_TIME[currCat]/TIMER_TICK, ledPinsCat[currCat]);
        lightLed(led, ledPinsCat[currCat]);

}


// Perform the recurrent task set by the timer function
// which update led only if the time is out, otherwise
// the timer function is stopped
void doRecurrentTask(){
        if(timerTick(CAT_0) >= 0){
                updateLedPinsCat(CAT_0);
        }
        if(timerTick(CAT_1) >= 0){
                updateLedPinsCat(CAT_1); 
        }  
        
        if(CAT_TIME[CAT_0] < 0 && CAT_TIME[CAT_1] < 0)
                t.stop(tickEvent);
}


// Turn on a specific led of a specific serie
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
