#include <LiquidCrystal.h>

#define TIME_MSG_LEN  11   // time sync to PC is HEADER and unix time_t as ten ascii digits
#define TIME_HEADER  255

const long SECONDS_IN_A_DAY = 3600;
const long HOURS_IN_A_DAY = 24;
const long SECONDS_IN_A_HOUR = 60;

LiquidCrystal lcd(7, 8, 9, 10, 11, 13);

const int tempSensorPin = A0;

const int setTimeButtonPin = 2;
const int setAlarmAndPlusButtonPin = 4;
const int minusButtonPin = 12;

unsigned long offSetTimeMilliSecs = 0;
unsigned long offSetHours = 0;
unsigned long offSetMins = 0;

int currentHours = 0;
int currentMins = 0;

enum state {
  state_none,
  state_setAlarmMin,
  state_setAlarmHour,
  state_setTimeHour,
  state_setTimeMin  
};

state currentState = state_none;

//-----------------------------------------------------------------------

void setup(){
  lcd.begin(16, 3);
  analogReference(EXTERNAL);
  Serial.begin(9600);
  
  pinMode(setTimeButtonPin, INPUT);  
  pinMode(setAlarmAndPlusButtonPin, INPUT);  
  pinMode(minusButtonPin, INPUT);  
}

//-----------------------------------------------------------------------

void loop() { 
  
  printInformationOnLcd();
  setTimeAndAlarm();
  delay(300);
}

//-----------------------------------------------------------------------

void printInformationOnLcd(){
  lcd.setCursor(0,0);
  lcd.print(getStringTime());  
  lcd.print(" ");
  lcd.print(getTemperature());
}

//-----------------------------------------------------------------------

void setTimeAndAlarm() {
  char buffer[20];
  changeState();
  while (currentState != state_none){
  int changeTimeAmount = digitalRead(setAlarmAndPlusButtonPin);
  changeTimeAmount -= digitalRead(minusButtonPin);
  
  Serial.println(digitalRead(setAlarmAndPlusButtonPin));
  Serial.println(digitalRead(minusButtonPin));
  
    switch (currentState) {
      case state_setTimeHour:
        lcd.setCursor(0,0);        
        offSetHours += changeTimeAmount;   
        if (offSetHours == HOURS_IN_A_DAY) offSetHours = 0;
        
        currentHours += changeTimeAmount;
        if (currentHours == HOURS_IN_A_DAY) currentHours = 0;
        
        sprintf(buffer, "%02d", currentHours);
        break;
      case state_setTimeMin:
        lcd.setCursor(3,0);        
        offSetMins += changeTimeAmount;  
        if (offSetMins == SECONDS_IN_A_HOUR) offSetMins = 0;    
        
        currentMins += changeTimeAmount;
        if (currentMins == SECONDS_IN_A_HOUR) currentMins = 0;   
        
        sprintf(buffer, "%02d", currentMins);
             
        break;
      case state_setAlarmHour:
        //currentState = state_setAlarmMin;
        break;
      case state_setAlarmMin:
        //currentState = state_none;
        break;        
  }  
  
  lcd.print(buffer); 
  lcd.cursor();
  changeState();
  delay(300);
  }
  lcd.noCursor();
  //setTimeHour();
}



void changeState(){
  int switchState = digitalRead(setTimeButtonPin);
  if (switchState) {
    switch (currentState) {
      case state_none:
        currentState = state_setTimeHour;
        break;
      case state_setTimeHour:
        currentState = state_setTimeMin;
        break;
      case state_setTimeMin:
        currentState = state_none;
        break;
      case state_setAlarmHour:
        currentState = state_setAlarmMin;
        break;
      case state_setAlarmMin:
        currentState = state_none;
        break;       
    } 
    while (switchState) {
      switchState = digitalRead(setTimeButtonPin);
    }
  }
  
  if (currentState == state_none && digitalRead(setAlarmAndPlusButtonPin)){
      currentState = state_setAlarmHour;
  }
  
  Serial.println(currentState);
  
}

//-----------------------------------------------------------------------

//http://forum.arduino.cc/index.php?topic=18588.0
String getStringTime()
{
   char buffer[20];
   unsigned long milliSeconds = millis() + offSetTimeMilliSecs;

   int secs  = milliSeconds / 1000; // secs is the total number of number of seconds
   int fracTime = milliSeconds % 1000; // fracTime the number of thousandths of a second  
   
   // number of days is total number of seconds divided by 24 divided by 3600
   int days = secs / (HOURS_IN_A_DAY*SECONDS_IN_A_DAY);
   secs = secs % (HOURS_IN_A_DAY*SECONDS_IN_A_DAY);

   // get the hours
   int hours = (secs / SECONDS_IN_A_DAY) + offSetHours;
   secs  = secs % SECONDS_IN_A_DAY;

   // get the minutes
   int mins = (secs / SECONDS_IN_A_HOUR) + offSetMins;
   secs = secs % SECONDS_IN_A_HOUR;

   sprintf(buffer, "%02d:%02d:%02d", hours, mins, secs);
   //lcd.print(buffer);
   currentHours = hours;
   currentMins = mins;
   return buffer;
}

//-----------------------------------------------------------------------

float getTemperature() {
  int sensorVal = analogRead(tempSensorPin);
  //Serial.println(sensorVal);
  float voltage = sensorVal * 3.3;
  voltage /= 1024.0; 
  float temprature = (voltage - 0.5)*100;
  return temprature;
}

