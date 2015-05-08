/*
  OmNomCat
  Servo control
 */

// include the servo library
#include <Servo.h>
// include the timer library
#include "Timer.h"
// include the LCD library:
#include <LiquidCrystal.h>

// initialize the LCD with the numbers of the interface pins
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

Servo myServo;  // create a servo object
Timer t;

int const SERVO_PIN = 9;
int const SWITCH_PIN = 13;
int const POT_PIN = A0;

int MAX_ANGLE = 179;
int MIN_ANGLE = 0;

int FOOD_TIME = 20;//ticks between each dispense
int TIMER_TICK = 1000;//ms between each "tick"
int FOOD_TIME_COUNTER = 20;

int tickEvent = 0;
int potVal;

//Custom designed character for the LCD
byte newChar[8] = {
        B11111,
        B10101,
        B11111,
        B10101,
        B10101,
        B11111,
        B10101,
        B11111
};

//Setup the arduino first
void setup() {
  Serial.begin(9600); 
  
  myServo.attach(SERVO_PIN); 
  tickEvent = t.every(TIMER_TICK, customTimer);
  
  lcd.createChar(7, newChar);
  lcd.begin(16, 2);

}

//Update the LCD with percentage value
void updateLCD(int percentage){
  Serial.println(percentage);
  //16 per line
  lcd.clear();//Clear the LCD screen
  int row = 0;
  while(row < 2){//Two rows (vertical bars)
    lcd.setCursor(0, row);//Set to bottom
    float i = 0;
    while(i < percentage){//6.25
      lcd.write(byte(7));//Write custom byte value
      i += (float)100 / (float)16;//Equal to 6.25
    }
    row++;
    }
}


void loop() {
  dispenseFoodButton();
  t.update();
  int potValNew = analogRead(POT_PIN);
  if(potVal != potValNew){//Value has changed - go in to a different mode!
    potVal = potValNew;
    changeTimer(potVal);
  }
}

void changeTimer(int potential){
  //Display a new value to the LCD
  int percentage = ((float)potential / (float)1024)*100;
  updateLCD(percentage);
  //Update globals
  FOOD_TIME = 20 + (percentage);
}

void timerTick(){
  FOOD_TIME_COUNTER = FOOD_TIME_COUNTER - 1;
  //Serial.println(FOOD_TIME_COUNTER);
  
  int percentage =  ((float)FOOD_TIME_COUNTER / (float)FOOD_TIME) * 100;
  updateLCD(percentage);
  
  if(FOOD_TIME_COUNTER == 0){
    dispenseFood();//OMNOMNOM
    FOOD_TIME_COUNTER = FOOD_TIME;//Reset time
  }
}


void customTimer(){
   resetTimer();
   timerTick();
}

void resetTimer(){
  t.stop(tickEvent);
  tickEvent = t.every(FOOD_TIME, customTimer);
}

//Dispense food from the machine.
//Turn full in both directions with delay to ensure the motor completes the motion
void dispenseFood(){
    myServo.write(MAX_ANGLE);    
    delay(1000);
    myServo.write(MIN_ANGLE);
    delay(1000);
}

//User activated button to skip the timer
//Check is button is pressed
//If it is start dispense
void dispenseFoodButton(){
  if (isButtonPressed()) {
    resetTimer();
    dispenseFood();
  }
  // wait for the servo to get there
  delay(300);
}

//Check if the button is pressed
boolean isButtonPressed(){
  return (digitalRead(SWITCH_PIN) == HIGH);
}
