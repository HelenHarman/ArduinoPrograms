#include <LiquidCrystal.h>

#define TIME_MSG_LEN  11   // time sync to PC is HEADER and unix time_t as ten ascii digits
#define TIME_HEADER  255

const long SECONDS_IN_A_DAY = 3600;
const long HOURS_IN_A_DAY = 24;
const long SECONDS_IN_A_HOUR = 60;

LiquidCrystal lcd(7, 8, 9, 10, 11, 13);

const int tempSensorPin = A0;

unsigned long offSetTimeMilliSecs = 0;

//-----------------------------------------------------------------------

void setup(){
  lcd.begin(16, 3);
  analogReference(EXTERNAL);
  Serial.begin(9600);
}

//-----------------------------------------------------------------------

void loop() { 
  
  printInformationOnLcd();
  delay(1000);
}

//-----------------------------------------------------------------------

void printInformationOnLcd(){
  lcd.setCursor(0,0);
  lcd.print(getStringTime());  
  lcd.print(" ");
  lcd.print(getTemperature());
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
   int hours = secs / SECONDS_IN_A_DAY;
   secs  = secs % SECONDS_IN_A_DAY;

   // get the minutes
   int mins = secs / SECONDS_IN_A_HOUR;
   secs = secs % SECONDS_IN_A_HOUR;

   sprintf(buffer, "%02d:%02d:%02d", hours, mins, secs);
   //lcd.print(buffer);
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

