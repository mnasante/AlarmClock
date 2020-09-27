#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <Fonts/Dialog_plain_10.h>
#include <DHT.h>
//#include <Time.h>
//#include <TimeLib.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels



#define DHTPIN 2     // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT11     // DHT 11
#define BLUELED 5
#define WHITELED 6
#define REDLED 11


const int beeper = 3; // Piezo Speaker pin D3
const int button1 = 10; // Button to change time units
const int button2 = 8; // Button to change the mode
const int button3 = 4; // Button to change LED mode
volatile int hour = 13;
volatile int minute = 0;
volatile int second = 0;
volatile int month = 10;
volatile int day = 1;
volatile int year;
boolean currentState = false;
boolean shouldListen = true;
int mode = 0;
int ledMode = 0;
int batteryVoltage = A0;
int rRef1 = 1000;
int rRef2 = 1000;
int sum = 0;
int brightness = 0;    // how bright the LED is
int brightness2 = 0;
int fadeAmount = 50;    // how many points to fade the LED by
int fadeAmount2 = 5;
float fullBattery = 9;
volatile int countHour = 13;

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

//Declaration for DHT Temperature sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {


  dht.begin();
  display.setFont(&Dialog_plain_10);
  pinMode(beeper, OUTPUT); // Setting up Piezo Speaker for beeping
  pinMode(button1, INPUT_PULLUP); // Setting up time button
  pinMode(button2, INPUT_PULLUP); // Setting up mode button
  pinMode(button3, INPUT_PULLUP); // Changing the LED mode

  pinMode(WHITELED, OUTPUT);
  pinMode(BLUELED, OUTPUT);
  pinMode(REDLED, OUTPUT);




  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  //Set initial paramters for display
//  delay(2000);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  cli();//stop interrupts

  //set timer1 interrupt at 1Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);


  sei();//allow interrupts







}

void fade()
{

  // change the brightness for next time through the loop:
  brightness = brightness + fadeAmount;
  

  // reverse the direction of the fading at the ends of the fade:
  if (brightness <= 0 || brightness >= 250) {
    fadeAmount = -fadeAmount;
  }
}

void fade2()
{
  
  // change the brightness for next time through the loop:
  brightness2 = brightness2 + fadeAmount2;
  

  // reverse the direction of the fading at the ends of the fade:
  if (brightness2 <= 0 || brightness2 >= 250) {
    fadeAmount2 = -fadeAmount2;
  }
}
/*
void fadePin(int start, int ends, int duration, int pin)
{
  int delta = ends-start;
  if (delta > 0)
  {
    int times = duration/delta;
    for (int i = start; i <= ends; i++)
    {
      analogWrite(pin, i);
      delay(times);
    }
  }
  else if (delta < 0)
  {
    int times = -1*duration/delta;
    for (int i = start; i >= ends; i--)
    {
      analogWrite(pin, i);
      delay(times);
    }
  }
}
*/

int brightOffset(int bright, int fade, int steps) {
  if (steps == 0){
    if (bright > 250){
      bright = 250;
    }
    else if (bright < 0){
      bright = 0;
    }
   return bright;
  }
  if (bright > 250 || bright < 0){
    fade *= -1;
  }
  return brightOffset(bright + fade, fade, steps - 1);
}



void showLights()
{


if (ledButton())
{
  tone(beeper,1000,100);
  delay(100);
  ledMode++;
}
    if (ledMode >= 7)
    {
      ledMode -= 7;
    }

    switch (ledMode)
    {
      case 0:
        analogWrite(WHITELED, 0);
        analogWrite(BLUELED, 0);
        analogWrite(REDLED, 0);
        fade2();
           
        break;

      case 1:
        analogWrite(WHITELED, brightness2);
        analogWrite(BLUELED, brightness2);
        analogWrite(REDLED, brightness2);
        fade2();
        break;

       case 2:
        analogWrite(WHITELED, brightness2);
        analogWrite(BLUELED, 0);
        analogWrite(REDLED, 0);
        fade2();
        break;

       case 3:
        analogWrite(WHITELED, 0);
        analogWrite(BLUELED, brightness2);
        analogWrite(REDLED, 0);
        fade2();
        break;

       case 4:
        analogWrite(WHITELED, 0);
        analogWrite(BLUELED, 0);
        analogWrite(REDLED, brightness2);
        fade2();
        break;

       case 5:
       analogWrite(WHITELED, brightOffset(brightness, fadeAmount,30));
       analogWrite(BLUELED, brightOffset(brightness, fadeAmount,35));
       analogWrite(REDLED, brightOffset(brightness, fadeAmount,40));
       fade();
       break;

       case 6:
        analogWrite(WHITELED, 250);
        analogWrite(BLUELED, 250);
        analogWrite(REDLED, 250);
        fade2();
        break;
    }

}

int timeButton()
{
  return digitalRead(button1) == LOW;
}

int modeButton()
{
  return digitalRead(button2) == LOW;
}

int ledButton()
{
  
  return digitalRead(button3) == LOW;
}

/**
   Show the battery level of a 9V battery on the display
*/

void displayBattery() {


display.drawCircle(121,6,6,WHITE);
display.fillCircle(119,4,1,WHITE);
display.fillCircle(123,4,1,WHITE);
display.drawPixel(119,8,WHITE);
display.drawPixel(120,9,WHITE);
display.drawPixel(121,9,WHITE);
display.drawPixel(122,9,WHITE);
display.drawPixel(123,8,WHITE);
/*
  float sum = 0;
  float currVoltage = (5 * analogRead(batteryVoltage) / 1023.0) * 2;


  // delay(1000);
  //  Serial.print(currVoltage);
  //  Serial.print("\n");

  display.drawRoundRect(110, 1, 18, 10, 2, WHITE); // Draw Empty battery logo (x,y, width, height, roundedness, color)

  //Show full Battery
  if (currVoltage >= 5.8)
  {
    display.fillRoundRect(122, 2, 4, 8, 2, WHITE);
    display.fillRoundRect(117, 2, 4, 8, 2, WHITE);
    display.fillRoundRect(112, 2, 4, 8, 2, WHITE);
  }

  //Show 2/3 battery
  else if (currVoltage >= 5.2 && currVoltage <= 5.8)
  {
    display.fillRoundRect(122, 2, 4, 8, 2, WHITE);
    display.fillRoundRect(117, 2, 4, 8, 2, WHITE);
  }

  //Show 1/3 battery
  else if (currVoltage >= 4.8 && currVoltage <= 5.2)
  {
    display.fillRoundRect(122, 2, 4, 8, 2, WHITE);
  }

  else if (currVoltage < 4.8)
  {
    display.drawRoundRect(110, 1, 18, 10, 2, WHITE);
  }
*/

}

/**
   Show an Encouraging Message each day
*/
void showEncMessage()
{
display.clearDisplay();
static int newMyDay1 = day+6;

 // display.setTextSize(1);


  //const char *encMessages[20] = {"You're awesome!", "You look good today!", "Have a great day!", "Go Olivia!", "Change lives today!", "Follow your dreams!", "Show Compassion!", "Dream big!", "Aim for Greatness!", "Be a better You!", "You are loved!", "Have fun today!", "You are Beautiful!", "Be Yourself!", "Jesus loves You!", "You are a Gift!", "You deserve a Hug!", "You are valuable!", "Stay funny!", "Today is your day!"};
   char *encMessages2[] = {"You are loved!", "Have fun today!", "Be Yourself!", "You look good today!", "Follow your dreams!"};
  char *encMessages3[3] = {"You look good today!", "Follow your dreams!"};
  const char *messages[] = {"You ", "lo", "ok ", "goo", "d to", "day!"};
  const char *messages2[] = {"You'", "re a", "weso", "me!"};
  const char *one[] = {"Y", "o", "u", "'", "r", "e", " ", "a", "w", "e", "s", "o", "m", "e", "!"};
  const char *two[] = {"Y", "o", "u", " ", "l", "o", "o", "k", " ", "g", "o", "o", "d", " ", "t", "o", "d", "a", "y", "!"};
  const char *three[] = {"H", "a", "v", "e", " ", "a", " ", "g", "r", "e", "a", "t", " ", "d", "a", "y", "!"};
  const char *four[] = {"G", "o", " ", "O", "l", "i", "v", "i", "a", "!"};
  const char *five[] = {"C", "h", "a", "n", "g", "e", " ", "l", "i", "v", "e", "s", " ", "t", "o", "d", "a", "y", "!"};

  //display.setCursor(1, 16);
  //display.print("Have a great daygreageg!");

  //  for (int i = 0; i <= day; i++)
  //{

  display.setTextSize(1);
  display.setCursor(1, 23);
  display.clearDisplay();

  
  display.print(encMessages2[day - 1]);
/*
display.print(encMessages2[day-1]);

if (day == 4)
{
  display.print(encMessages3[1]);
}
*/
  /*display.print(messages[day]);
  display.print(messages[day+1]);
  display.print(messages[day+2]);
  display.print(messages[day+3]);
  display.print(messages[day+4]);
  display.print(messages[day+5]);
  */
/*
display.clearDisplay();
if (mode == 1) display.print(encMessages2[1]);
if (mode == 2) {display.print(""); display.print(encMessages2[2]);}
if (mode == 3) {display.print(""); display.print(encMessages2[3]);}
if (mode == 4){ display.print(""); display.print(encMessages2[4]);}
if (mode == 5) {display.print(""); display.print(encMessages2[5]);  }
*/
/*

  switch(mode)
  {
    case 1:
    display.clearDisplay();
    display.print(encMessages2[1]);
    break;

    case 2:
    display.clearDisplay();
    display.print(encMessages2[2]);
    break;

    case 3:
    display.clearDisplay();
    display.print(encMessages2[3]);

    case 4:
    display.clearDisplay();
    display.print(encMessages2[4]);
    break;

    case 5:
    display.clearDisplay();
    display.print(encMessages2[5]);
    break;
    
  }
*/
  /*
  if (day == 1) { display.clearDisplay(); display.print("You "); display.print("lo"); display.print("ok "); display.print("goo"); display.print("d to"); display.print("day!");}
  else if (day == 2) { display.clearDisplay(); display.print("You'"); display.print("re a"); display.print("weso"); display.print("me!"); }
  else if (day == 3) { display.clearDisplay(); display.print("Hav"); display.print("e a "); display.print("grea"); display.print("t "); display.print("day!");}
  else if (day == 4) { display.clearDisplay(); display.print("Go O"); display.print("livi"); display.print("a!"); }
  else if (day == 5) { display.clearDisplay(); display.print("Chan"); display.print("ge "); display.print("live"); display.print("s"); display.print(" tod"); display.print("ay!");}
  */
 // display.print(encMessages2[3]);
  // display.setCursor(120, 16);
  //  display.print("FFF");

/*
if (day == 1) display.print("You're awesome!");
else if (day == 2) display.print("You look good Today!");
else if (day == 3) display.print("Have a great day!");
else if (day == 4) display.print("Go Olivia!");
else if (day == 5) display.print("Change lives today!");
else if (day == 6) display.print("Follow your dreams!");
else if (day == 7) display.print("Show compassion!");
*/
  // }



  
  
}

/**
   Show the current Temperature and Humidity
*/

void showTemp()
{

  //read temperature and humidity
  float tC = dht.readTemperature();
  int tF = (dht.readTemperature() * 1.8) + 32;
  int h = dht.readHumidity();
  if (isnan(h) || isnan(tF)) {
    Serial.println("Failed to read from DHT sensor!");
  }




  // display Temperature in Degrees Fahrenheit
  display.setTextSize(1);
  display.setCursor(0, 38);
  display.print("Temp: ");
  display.setTextSize(2);
  display.setFont();
  display.setCursor(80, 30);
  display.print(tF);
  display.cp437(true); // Show the degrees symbol
  display.write(167);
  display.print("F");
  display.setFont(&Dialog_plain_10);

  //Display Humidity
  display.setTextSize(1);
  display.setCursor(0, 60);
  display.print("Humidity:");
  display.setTextSize(2);
  display.setFont();
  display.setCursor(80, 50);

  display.print(h);
  display.print("%     ");
  display.setFont(&Dialog_plain_10);
  display.setTextColor(WHITE, BLACK); // Refreshes the Temperature and Humidity portion of the display each second
  

}

/**
   Print time digits in format - hh:mm:ss
*/
void printDigits(int digits) {
  // utility function for digital clock display: prints preceding colon and leading 0
  display.print(":");
  if (digits < 10)
    display.print('0');
  display.print(digits);
}

void printHourDigits(int digits)
{

  if (digits < 10)
    display.print(" ");
  display.print(digits);
}

void printDayDigits(int digits) {
  // utility function for digital clock display prints preceding forward and leading 0
  display.print("/");
  if (digits < 10)
    display.print('0');
  display.print(digits);
}

void printMonthDigits(int digits)
{

  if (digits < 10)
    display.print('0');
  display.print(digits);
}


ISR(TIMER1_COMPA_vect) { //timer1 interrupt 1Hz toggles pin 13 (LED)


  
  second = second + 1;
}


/**
   Sets whether it's PM or AM
*/
boolean isPM(int hr)
{
  if (hr < 12)
  {
    return false;
  }

  else if (hr >= 12)
  {
    return true;
  }
  //Default is false, which means it's AM
}


/**
   Definitions for setting the time
*/
void setTime(unsigned int myHour, volatile int myMinute, volatile int mySecond, int myMonth, volatile int myDay, int myYear)
{



  hour = myHour;
  minute = myMinute;
  second = mySecond;
  day = myDay;
  month = myMonth;


  //If number of seconds exceeds 60, reset to 00 and increment minute by 1
  while (second >= 60)
  {
    minute = minute + 1;
    second = second - 60;


  }

  //If number of minutes exceeds 60, reset to 00 and increment hour by 1
  while (minute >= 60)
  {
    minute = minute - 60;
    hour++;
    countHour++;
  }



  if (isPM(countHour) == true)
  {
    display.setCursor(55, 9);
    display.setTextSize(1);
    display.print("PM");
  }


  if (isPM(countHour) == false)
  {
    display.setCursor(55, 9);
    display.setTextSize(1);
    display.print("AM");

  }



  while (hour > 12)
  {
    hour = hour - 12;
  }

  while (countHour == 24)
  {
    countHour = countHour - 24;
    day++;
  }


  timeButtons();

}

void day28()
{
  month++;
  day -= 28;
}

void day30()
{
  month++;
  day -= 30;
}

void day31()
{
  month++;
  day -= 31;
}

/**
 * Sets months to their corresponding days
 */
void monthFunction(volatile int myMonth, volatile int myDay)
{


  switch (myMonth)
  {
    case 1:
      if (myDay > 31)
        day31();
      break;

    case 2:
      if (myDay > 28)
        day28();
      break;

    case 3:
      if (myDay > 31)
        day31();
      break;

    case 4:
      if (myDay > 30)
        day30();
      break;

    case 5:
      if (myDay > 31)
        day31();
      break;

    case 6:
      if (myDay > 30)
        day30();
      break;

    case 7:
      if (myDay > 31)
        day31();
      break;

    case 8:
      if (myDay > 31)
        day31();
      break;

    case 9:
      if (myDay > 30)
        day30();
      break;

    case 10:
      if (myDay > 31)
        day31();
      break;

    case 11:
      if (myDay > 30)
        day30();
      break;

    case 12:
      if (myDay > 31)
        day31();
      break;
  }
  while (month > 12)
  {
    month -= 12;
  }
}

/**
 * Definitions for what each time button does
 */
void timeButtons()
{
  if (modeButton())
  {
    tone(beeper,800,100);
    delay(100);
    mode++;
    if (mode >= 5)
    {
      mode -= 5;
    }

    switch (mode)
    {
      case 1:
      display.drawLine(21,11,33,11,WHITE);
      break;

      case 2:
      display.drawLine(4,11,16,11,WHITE);
      break;

      case 3:
      display.drawLine(95,11,105,11,WHITE);
      break;
      
      case 4:
      display.drawLine(77,11,87,11,WHITE);
      break;
    }

  }
  if (timeButton())
  {
    delay(100);

    switch (mode)
    {
      //Mode 1 = Change Minute
      case 1:
      
        minute++;
        if (minute >= 60)
          minute = minute - 60;
        break;

      //Mode 2 = Change Hour
      case 2:
        hour++;
        countHour++;
        break;

      //Mode 3 = Change Day
      case 3:
        day++;
        break;

      //Mode 4 = Change Month
      case 4:
        month++;
        break;
    }

  }

}


/**
 * Displays the time and date to the OLED display
 */
void displayTime()
{

  setTime(hour, minute, second, month, day, 2020);

  display.setTextSize(1);
  display.setCursor(3, 9);
  printHourDigits(hour);
  display.setCursor(16, 9);
  printDigits(minute);
  printDigits(second);

  display.setCursor(79, 9);
  printMonthDigits(month);

  display.setCursor(92, 9);
  printDayDigits(day);


}

void loop() {
  monthFunction(month, day);
  showLights();
  
     showEncMessage();
 
  displayBattery();
  
  displayTime();
   showTemp();

  display.display();

   

}
