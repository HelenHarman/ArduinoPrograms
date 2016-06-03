#include <LiquidCrystal.h>

#define TIME_MSG_LEN  11   // time sync to PC is HEADER and unix time_t as ten ascii digits
#define TIME_HEADER  255

#define SET_TIME_BUTTON_PIN 2
#define SET_ALARM_PLUS_BUTTON_PIN 4
#define MINUS_BUTTON_PIN 12
#define TEMPERATURE_SENSOR_PIN A0

LiquidCrystal lcd(7, 8, 9, 10, 11, 13);

const long SECONDS_IN_A_DAY = 3600;
const long HOURS_IN_A_DAY = 24;
const long SECONDS_IN_A_HOUR = 60;

int offSetHours = 0;
int offSetMins = 0;

int currentHours = 0;
int currentMins = 0;

int alarmHour = 0;
int alarmMin = 0;

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
  Serial.begin(9600);
  
  analogReference(EXTERNAL);//for temperature sensor, otherwise random values

  pinMode(SET_TIME_BUTTON_PIN, INPUT);  
  pinMode(SET_ALARM_PLUS_BUTTON_PIN, INPUT);  
  pinMode(MINUS_BUTTON_PIN, INPUT);  
}

//-----------------------------------------------------------------------

void loop() { 
  printInformationOnLcd();
  setTimeAndAlarm();
  delay(300);
}

//-----------------------------------------------------------------------

/**
 * Print the time and the temperature
 */
void printInformationOnLcd(){
  lcd.setCursor(0,0);
  lcd.print(getStringTime());  
  lcd.print("  ");
  lcd.print(getTemperature());
  
  lcd.setCursor(0,1);
  lcd.print(getGreeting());
}



//-----------------------------------------------------------------------

//http://forum.arduino.cc/index.php?topic=18588.0
String getStringTime()
{
  char buffer[20];
  unsigned long milliSeconds = millis();// + offSetTimeMilliSecs;

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
 
  currentHours = hours;
  currentMins = mins;
  return buffer;
}

//-----------------------------------------------------------------------

float getTemperature() {
  int sensorVal = analogRead(TEMPERATURE_SENSOR_PIN);
  //Serial.println(sensorVal);
  float voltage = sensorVal * 3.3; // yes I could use const, but nice to keep all temperature related stuff compacted to here.
  voltage /= 1024.0; 
  float temprature = (voltage - 0.5)*100;
  return temprature;
}

//-----------------------------------------------------------------------

String getGreeting() {
  if (currentHours < 4 || currentHours > 22) {
    return "Goedenacht";
  } else if (currentHours < 12) {
      return "Goedemorgen";
  } else if (currentHours < 18) {
    return "Goedenmiddag";
  } else if (currentHours < 22) {
    return "Goedenavond";
  } 
}

//-----------------------------------------------------------------------

//======== Code to set the alarm and the Time ===========//

//-----------------------------------------------------------------------

void setTimeAndAlarm() {
  changeState();
  while (currentState != state_none){
    lcd.cursor();
    
    switch (currentState) {      
    //----- set time ------//
    case state_setTimeHour:
      lcd.setCursor(0,0);        
      changeTimeValue(&currentHours, &offSetHours, HOURS_IN_A_DAY);
      break;
    case state_setTimeMin:
      lcd.setCursor(3,0);        
      changeTimeValue(&currentMins, &offSetMins, SECONDS_IN_A_HOUR);
      break;
      
    //----- set alarm ------//
    case state_setAlarmHour:
      lcd.setCursor(0,0); 
      changeTimeValue(&alarmHour, NULL, HOURS_IN_A_DAY);
      break;
    case state_setAlarmMin:
      lcd.setCursor(3,0);
      changeTimeValue(&alarmMin, NULL, SECONDS_IN_A_HOUR);
      break;        
    }  
   
    changeState();
    delay(300);
  }
  lcd.noCursor();
}

//-----------------------------------------------------------------------

void changeTimeValue(int* currentTime, int* offSetTime, int maxValue){
  char buffer[20];
  int changeTimeAmount = digitalRead(SET_ALARM_PLUS_BUTTON_PIN);
  changeTimeAmount -= digitalRead(MINUS_BUTTON_PIN);

  //----- update the off set --------//
  if (offSetTime) {
    *offSetTime += changeTimeAmount;   
    checkTimeValueInRange(offSetTime, maxValue);
  }

  //----- update the current time and display it --------//
  *currentTime += changeTimeAmount;
  checkTimeValueInRange(currentTime, maxValue);

  sprintf(buffer, "%02d", *currentTime);
  lcd.print(buffer);
}

//-----------------------------------------------------------------------

void checkTimeValueInRange(int* timeValue, int maxValue) {
  if (*timeValue == maxValue) { // so that the max hours/mins is 23/59
    *timeValue = 0;
  } 
  else if (*timeValue == -1) { // so that the min hours/mins is 0
    *timeValue = maxValue-1;
  }
}

//-----------------------------------------------------------------------

void changeState(){
  int switchState = digitalRead(SET_TIME_BUTTON_PIN);
  
  // if first button clicked change between setting time and alarm's hours and mins
  if (switchState) {
    switch (currentState) {
    //------- set time ------//
    case state_none:
      currentState = state_setTimeHour;
      break;
    case state_setTimeHour:
      currentState = state_setTimeMin;
      break;
    case state_setTimeMin:
      currentState = state_none;
      break;
      
    //----- set alarm time ------//
    case state_setAlarmHour:
      currentState = state_setAlarmMin;
      break;
    case state_setAlarmMin:
      currentState = state_none;
      break;       
    } 
    
    // wait until button is no longer being held down
    while (switchState) {
      switchState = digitalRead(SET_TIME_BUTTON_PIN);
    }
  }

  // second button clicked with state is none means user wants to set the alarm
  switchState = digitalRead(SET_ALARM_PLUS_BUTTON_PIN);
  if (currentState == state_none && switchState){
    currentState = state_setAlarmHour;
    while (switchState) { // wait until button is no longer being held down
      switchState = digitalRead(SET_TIME_BUTTON_PIN);
    }
  }
}

//-----------------------------------------------------------------------
//-----------------------------------------------------------------------

