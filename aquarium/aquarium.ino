/*
 
 Aquarium controler
 
 (C) 5/2014 Flavius Bindea

=============================================== 
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
===============================================

*/
 
 
#include <Wire.h> // include the I2C library
#include <RTClib.h> // From: https://github.com/adafruit/RTClib.git 573581794b73dc70bccc659df9d54a9f599f4260
#include <EEPROM.h> // Fro read and write EEPROM
#include <DHT.h>
DHT dht;
// include the library code:
#include <LiquidCrystal.h>

// used for RTC
const int dayspermonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
RTC_DS1307 RTC;
DateTime now;

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(13, 12, 5, 4, 3, 2);

// 126: -> 127: <-
byte up[8] = {
  B00100,
  B01110,
  B10101,
  B00100,
  B00100,
  B00100,
  B00100,
};

byte down[8] = {
  B00100,
  B00100,
  B00100,
  B00100,
  B10101,
  B01110,
  B00100,
};

// For Temperature sensor TMP36 on A0
// Change values depending on your real measurements
//#define aref_voltage 4.91 // real voltage
//#define amplifier 3.27    // 3.27 -> amplifier = (R8+R10)/R8 = (220+500)/220, exact=(216+558)/216=3.58
 #include "pitches.h"

// notes in the melody:
int melody[] = {
  NOTE_C4, NOTE_G3,NOTE_G3, NOTE_A3, NOTE_G3,0, /*NOTE_B3, NOTE_C4*/};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4,4,4/*,4,4*/ };
  
//const float baselineTemp = 20.0;
const int sensorPin = A0;

// For Averaging
// Define the number of samples to keep track of.  The higher the number,
// the more the readings will be smoothed, but the slower the output will
// respond to the input.  Using a constant rather than a normal variable lets
// use this value to determine the size of the readings array.
/*#define numReadings 1
int readings[numReadings];      // the readings from the analog input
int index = 0;                  // the index of the current reading
int total = 0;                  // the running total
int average = 0;                // the average
int full = 0;                   // boolean in order to know if we have enoungh measurements
*/
// For buttons
const int buttonsPin = A1;
int bstate = 1024, blast = 1024;  // button state and button last state

// change value depending on your measurements
const int button1max = 75;    // reading should be 0, 75 threshold
const int button2min = 76;   // reading should be 151, from 76 to 250
const int button2max = 250;
const int button3min = 251;   // reading should be 347, from 251 to 430
const int button3max = 430;
const int button4min = 431;   // reading should be 515, from 431 to 606
const int button4max = 606;
const int button5min = 607;   // reading should be 700, from 607 to 850
const int button5max = 850;
const int buttonNONE = 900;   // reading should be 1023

// button types
#define BT_NONE 0
#define BT_SET 1
#define BT_LEFT 2
#define BT_RIGHT 3
#define BT_UP 4
#define BT_DOWN 5

// For looping display by interval
unsigned long previousDisplayMillis = 0; 
unsigned long displayInterval = 1000; 
// For looping calculation by interval
unsigned long previousCalculationMillis = 0; 
unsigned long calculationInterval = 250;

// screen size
const byte cols = 16, lines = 2;

// menu of status
const int menumin = 0;
const int menumax = 3;

char* menu_entry[] = {
  "1. Temperatura ",
  "2. Umidita'    ",
  "3. Luci        ",
  "4. Set Data/Ora"
  //"5. Menu entry 5 ",
  //"6. Menu vuoto 6 "
};

// status of programm
#define ST_DISPLAY 0
#define ST_MENU 1
int status = ST_DISPLAY;

/*
 * function prototypes
 */
void set_function(byte lnb, byte wpower=1);

/*
 * Define the devices
 */
#define ventola 10
#define serpentina 11
#define umidita 9
#define luci 8
//#define Humidity_Led 7
//#define Temp_Led 6
struct AQTIME {
  byte h1;
  byte m1;
  byte h2;
  byte m2;
  byte power;
  //byte temp;
};

// number of setups in memory
#define NBSETS 4
AQTIME ti[NBSETS];
byte out[NBSETS];

// statuses of outputs
#define OFF 0
#define AUTO 1
#define ON 2
//#define MAX 3
byte out_m[NBSETS];

// for nice transition
//const unsigned long transitionDuration = 0000;
//unsigned int transitionSteps;
byte asked_l[NBSETS]; // new asked level
byte last_l[NBSETS];  // last asked level
unsigned int current_l[NBSETS]; // current level multiplied by 256 in order to avoid floating calculations
int incr_l[NBSETS];   // step increment level multiplied by 256 in order to avoid floating calcultations

#define LightSet 0
#define SwitchSet 2

// EEPROM signature for aquarium: they are stored in 0 and 1
const byte AQ_SIG1 = 45, AQ_SIG2 = 899;

// Initial setup
void setup() 
{
  Serial.begin(57600);
  Serial.println("Welcome to Aquarium Controler");
  dht.setup(A0);                          // sostituito sensorpin
  delay(dht.getMinimumSamplingPeriod());  // nostro
  pinMode(sensorPin, INPUT);
  // Configures RTC
  Wire.begin();                           // initalise I2C interface  
  
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }

  // If you want to set the aref to something other than 5v
  //analogReference(EXTERNAL);

  // Configures display
  // set up the number of columns and rows on the LCD 
  lcd.createChar(1, up);
  lcd.createChar(2, down);

  lcd.begin(cols, lines);
  
  // Print a message to the LCD.
  lcd.print("Reptile");
  // set the cursor to column 0, line 1
  // line 1 is the second row, since counting begins with 0
  lcd.setCursor(0, 1);
  // print to the second line
  lcd.print("Controller");
  delay(800);
  
  // trys to read EEPROM
  if(AQ_SIG1 != EEPROM.read(0) || AQ_SIG2 != EEPROM.read(1)) {
    lcd.print(" NOT SET");
    delay(2000);
    EEPROM.write(0, AQ_SIG1);
    EEPROM.write(1, AQ_SIG2);
    
    for(int i = 2; i < 2+NBSETS*5; i++) {
      EEPROM.write(i, 0);
    }
  }
  else {
    // reads the EEPROM setup
    read_eeprom(0);
    read_eeprom(1);
    read_eeprom(2);
    read_eeprom(3);
  }
  
  // setout leds
  pinMode(umidita, OUTPUT);
  pinMode(luci, OUTPUT);
  pinMode(ventola, OUTPUT);
  pinMode(serpentina, OUTPUT);
  //pinMode(Humidity_Led, OUTPUT); // al momento scollegato
  //pinMode(Temp_Led, OUTPUT); 
  // Set initial state, tutto spento tranne status led
  digitalWrite(umidita, LOW);
  digitalWrite(luci, LOW);
  analogWrite(ventola, 0);    // Turn off light 1
  analogWrite(serpentina, 0);    // Turn off light 2
  //digitalWrite(Humidity_Led, LOW);
  out[0] = ventola;
  out[1] = serpentina;
  out[2] = umidita;
  out[3] = luci;
  for(int i = 0; i < NBSETS; i++) {
    out_m[i] = AUTO;
    //current_l[NBSETS] = asked_l[NBSETS] = last_l[NBSETS] = 0;  // last asked level and last level
  }    

  // smooth transition
  //transitionSteps = transitionDuration / calculationInterval *transitionDuration;
  
  delay(50);
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 6; thisNote++) {

    // to calculate the note duration, take one second 
    // divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000/noteDurations[thisNote];
    tone(7, melody[thisNote],noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(7);
  }
}

/*
** Main loop
*/
void loop() 
{
  int pressed_bt;
 
  // For interval determination
  unsigned long currentMillis = millis();

  if(currentMillis - previousCalculationMillis > calculationInterval) {
      // save lasted calculation millis
      previousCalculationMillis = currentMillis;  
      // does interval calculations
      calculations();
  }
  if(status == ST_DISPLAY) {
    // only once an interval
    if(currentMillis - previousDisplayMillis > displayInterval) {
      //Serial.println("display interval");
      // save lasted display millis
      previousDisplayMillis = currentMillis;  
      // display the data on the screen
      display_data();
    } 
  }
  
  pressed_bt = read_button();

  switch(pressed_bt) {
    case BT_SET:
      chg_status();
      do_menu();
      break;
    case BT_LEFT:
      switch_out(0);
      break;
    case BT_RIGHT:
      switch_out(1);
      break;
   case BT_UP:
      switch_out(2);
      break;
   case BT_DOWN:
      switch_out(3);
      break;
   }
      
   // small delay
   delay(50);
}

// switch the menu status
void chg_status()
{
  if(status == ST_DISPLAY) {
    lcd.blink();
    status = ST_MENU;
  }
  else {
    lcd.noBlink();
    status = ST_DISPLAY;
  }
}

// switch out put mode
void switch_out(byte n)
{
  switch(out_m[n]) {
    case ON:
      out_m[n] = AUTO;
      break;
    case AUTO:
      out_m[n] = OFF; // 1
      break;
    /*case ON:
      out_m[n] = MAX;
      break;*/
    case OFF:
      out_m[n] = ON;
      break;
  }
  display_out(n);
}

/*
** return button status if it has changed
*/
int read_button()
{
  int button;
  /*
  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH),  and you've waited
  // long enough since the last press to ignore any noise:  

  // If the switch changed, due to noise or pressing:
  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed
    */
  // read the buttons
  button = analogRead(buttonsPin);

  blast = bstate;

  if (button < button1max)
    bstate = 1;
  else if (button >= button2min && button <= button2max)
    bstate = 2;
  else if (button >= button3min && button <= button3max)
    bstate = 3;
  else if (button >= button4min && button <= button4max)
    bstate = 4;
  else if (button >= button5min && button <= button5max)
    bstate = 5;
  else if (button >= buttonNONE)
    bstate = 0;
  else
    bstate = 99; // we should never arrive here

//  Serial.print("VALUE: "); Serial.println(button);

  if(bstate == 99) {
    Serial.print("ERROR: "); Serial.println(button);
  }
  
  if (blast != bstate) {
    // state has changed
    if(bstate >=1 && bstate <= 5) {
      Serial.print("BUTTON: "); Serial.println(bstate);  
      return(bstate);
    }
  }
  return(0);
}

// read blocking
int read_button_blocking()
{
  int i;
  // Serial.println("read button blocking");

  while((i = read_button()) == 0)
    delay(50);
    
  return i;
}

/*
** does interval calculations
*/

void calculations()
{
  int h, m;
  //Serial.println("calculations");

  // getting the voltage reading from the temperature sensor
  // subtract the last reading:
  //total= total - readings[index];        
  // read from the sensor:  
  //delay(100);
  //readings[index] = analogRead(sensorPin);
  //  delay(100);
  //Serial.print(readings[index]); Serial.println(" reading");
  // add the reading to the total:
  //total= total + readings[index];      
  // advance to the next position in the array:  
  //index = index + 1;                    

  //if (full == 0 && index == numReadings)
  //   full = 1;
     
  // if we're at the end of the array...
  //if (index >= numReadings)              
  // ...wrap around to the beginning:
  //  index = 0;                          

  //if(full) {
    // calculate the average:
    //average = total / numReadings;        
    //Serial.print(average); Serial.println(" average");
  
    // converting that reading to voltage
    //float voltage = average * aref_voltage/amplifier;
    //voltage /= 1024.0;
    // print out the voltage
    //Serial.print(voltage, 4); Serial.println(" volts");
    // now print out the temperature
    //temperatureC = (voltage - 0.5) * 100 ; //converting from 10 mv per degree wit 500 mV offset
    //Serial.print(temperatureC); Serial.println(" degrees C");
  //}
  //else {
  //Serial.print(index); Serial.println(" averaging");
  //}  

  // read the date  
  now = RTC.now();
  h = now.hour();
  m = now.minute();
  
  // setting the status of the outputs
  for(int li = 0; li < 4; li++) {
//    Serial.print("Calculation for ");
//    Serial.println(li);
//    Serial.print("Nb of steps:");
//   Serial.println(transitionSteps);

    byte out_s;

    if(out_m[li] == OFF)
      out_s = OFF;
    else if(out_m[li] == ON)
      out_s = ON;
    // else if(out_m[li] == MAX)
    //  out_s = MAX;
    else if (out_m[li] == AUTO) {
      if (li < 1) {
        int temperature  = dht.getTemperature()-6;
        if (ti[0].power < temperature ()) {
          // checking if we are in the ON time period
          byte order = ((ti[0].h2 > ti[0].h1) || (ti[0].h1 == ti[0].h2 && ti[0].m2 >= ti[0].m1)) ? 1 : 0;
          if ( order && (h > ti[0].h1 || (h == ti[0].h1 && m >= ti[0].m1)) && (h < ti[0].h2 || (h == ti[0].h2 && m <= ti[0].m2)) || ((h > ti[0].h2 || (h == ti[0].h2 && m >= ti[0].m2)) && (h < ti[0].h1 || (h == ti[0].h1 && m <= ti[0].m1))) ){
            out_s = ON;
          } else {
            out_s = OFF;
          }
        } else {
          out_s = OFF;
        }      
     } else if (li < 2) {
      int temperature  = dht.getTemperature()-6;
      if (ti[0].power > temperature) {
      // checking if we are in the ON time period
          byte order = ((ti[0].h2 > ti[0].h1) || (ti[0].h1 == ti[0].h2 && ti[0].m2 >= ti[0].m1)) ? 1 : 0;
          if ( order && (h > ti[0].h1 || (h == ti[0].h1 && m >= ti[0].m1)) && (h < ti[0].h2 || (h == ti[0].h2 && m <= ti[0].m2)) || ((h > ti[0].h2 || (h == ti[0].h2 && m >= ti[0].m2)) && (h < ti[0].h1 || (h == ti[0].h1 && m <= ti[0].m1))) ){
            out_s = ON;
          } else {
            out_s = OFF;
          }
        } else {
          out_s = OFF;
        }      
     } else if (li < 3) {
      int humidity  = dht.getHumidity()+20;
      
      if (ti[1].power > humidity) {
      // checking if we are in the ON time period
          byte order = ((ti[1].h2 > ti[1].h1) || (ti[1].h1 == ti[1].h2 && ti[1].m2 >= ti[1].m1)) ? 1 : 0;
          if ( order && (h > ti[1].h1 || (h == ti[1].h1 && m >= ti[1].m1)) && (h < ti[1].h2 || (h == ti[1].h2 && m <= ti[1].m2)) || ((h > ti[1].h2 || (h == ti[1].h2 && m >= ti[1].m2)) && (h < ti[1].h1 || (h == ti[1].h1 && m <= ti[1].m1))) ){
            out_s = ON;
          } else {
            out_s = OFF;
          }
        } else {
          out_s = OFF;
        }      
     } else if (li < 4) {
      //if (m < ti[3].m1) {
            // checking if we are in the ON time period
      byte order = ((ti[2].h2 > ti[2].h1) || (ti[2].h1 == ti[2].h2 && ti[2].m2 >= ti[2].m1)) ? 1 : 0;
      if ((h > ti[2].h1 || (h == ti[2].h1 && m >= ti[2].m1)) && (h < ti[2].h2 || (h == ti[2].h2 && m <= ti[2].m2)) || ((h > ti[2].h2 || (h == ti[2].h2 && m >= ti[2].m2)) && (h < ti[2].h1 || (h == ti[2].h1 && m <= ti[2].m1))) ) {
          out_s = ON; 
      } else {
        out_s = OFF;
      }
    }
  }
    /* 
    if(li < 2) {
//      Serial.print("Status = ");
//      Serial.println(out_s);
      switch(out_s) {
        case OFF:
          asked_l[li] = 0;
          break;
        /*case ON:
          asked_l[li] = ti[li].power*255/99;
          break;*/
      /*  case ON:
          asked_l[li] = 255;
          break;
      }*/
//      Serial.print("Asked Level = ");
//      Serial.print(asked_l[li]);
//      Serial.print(", Last Level = ");
//      Serial.print(last_l[li]);
/*
      if(asked_l[li] != last_l[li]) {
        incr_l[li] = ((long)asked_l[li]*256 - current_l[li])/transitionSteps;
        Serial.print("Set Increment To= ");
        Serial.println(incr_l[li]);
        last_l[li] = asked_l[li];
      }*/
//      Serial.print(", Increment = ");
//      Serial.print(incr_l[li]);
    
//      Serial.print(", Current Before = ");
//      Serial.println(current_l[li]);
      /*
      if(current_l[li] != asked_l[li]) {
        current_l[li] += incr_l[li];
        if(abs(current_l[li] - asked_l[li]*256) < abs(incr_l[li])) {
            Serial.println("Last--------------------------------");
             current_l[li] = (unsigned)asked_l[li]*256;          
             incr_l[li] = 0;
        }
      }*/
//      Serial.print(", Current After = ");
//      Serial.println(current_l[li]);
//      analogWrite(out[li], current_l[li]/256);
//    }
//    else 
   // for (li=0; li < 4; li++) {
      if(out_s == OFF)
        digitalWrite(out[li], LOW);
      else
        digitalWrite(out[li], HIGH);
    //}
  }
}

// does the menu
void do_menu()
{
  int pressed_bt = -1;
  int menuline = 0;

  Serial.println("do menu---------------------------------");

  start_menu();

  do {
    //Serial.print("not set button");
    //Serial.print("button = ");
    //Serial.print(pressed_bt);
    //Serial.print(",  menuline = ");
    //Serial.println(menuline);

    switch(pressed_bt) {
      case BT_LEFT:
        break;
      case BT_RIGHT:
        do_menu_entry(menuline);
        break;
     case BT_UP:
        menuline--;
        break;
     case BT_DOWN:
        menuline++;
        break;
     }
     if(menuline < menumin)
       menuline = menumin;
     else if(menuline > menumax)
       menuline = menumax;

    lcd.setCursor(0, 1);
    lcd.write(menu_entry[menuline]);
    lcd.print("\076");
    lcd.setCursor(15, 1);
  } while ((pressed_bt = read_button_blocking()) != BT_SET);

  Serial.println("SET button pressed")
;  chg_status();
}

void start_menu()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write("Menu:  ");
  lcd.write(1);
  lcd.write(2);
  lcd.write(' ');
  lcd.write(126);
  lcd.write("OK");
}

void do_menu_entry(int en)
{ 
  Serial.print("Do menu entry:");
  Serial.println(en);

  switch(en) {
     case 0:
       //set_time();
      set_function(1);
       break;
     case 1:
       set_function(2);
       break;
     case 2:
       set_function(3, 0);
       break;
     case 3:
       set_time();
       break;
    // case 4:
    // set_time();
    //   break;
    //   case 5:
    //   set_function(5, 0);
    //   break;
    //   case 6:
    //   set_function(6, 0);
    //  break;
  }
}

/*
** Menu entry to setup the time
*/
void set_time()
{
  int pressed_bt = -1;
  int pos = 0, v;
  char val[16];
  int day, month, year, hour, min;
  int i;
  int ok = 0;

  Serial.println("do set time---------------------------------");
  /*
  ** 0123456789012345
  ** 0         1
  ** DD/MM/YYYY HH:MM
  */
  
  now = RTC.now();
  val[0] = now.day()/10+'0';
  val[1] = now.day()%10+'0';
  val[2] = '/';
  val[3] = now.month()/10+'0';
  val[4] = now.month()%10+'0';
  val[5] = '/';
  year = now.year();
  val[6] = '2';
  val[7] = '0';
  year = year-2000;
  val[8] = year/10+'0';
  val[9] = year%10+'0';
  val[10] = ' ';
  val[11] = now.hour()/10+'0';
  val[12] = now.hour()%10+'0';
  val[13] = ':';
  val[14]= now.minute()/10+'0';
  val[15]= now.minute()%10+'0';

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write("Time: use");
  lcd.write(1);
  lcd.write(2);
  lcd.write(127);
  lcd.write(126);
  lcd.write("SET");

  lcd.setCursor(0, 1);
  for(i = 0; i < 16; i++)
    lcd.print(val[i]);

  do {
    do {
      Serial.println("not set button");
      switch(pressed_bt) {
        case BT_LEFT:
          switch(pos) {
              case 1:
                pos = 0;
                break;
              case 3:
                pos = 1;
                break;
              case 4:
                pos = 3;
                break;
              case 8:
                pos = 4;
                break;
              case 9:
                pos = 8;
                break;
              case 11:
                pos = 9;
                break;
              case 12:
                pos = 11;
                break;
              case 14:
                pos = 12;
                break;
              case 15:
                pos = 14;
                break;
          }
          break;
        case BT_RIGHT:
          switch(pos) {
              case 0:
                pos = 1;
                break;
              case 1:
                pos = 3;
                break;
              case 3:
                pos = 4;
                break;
              case 4:
                pos = 8;
                break;
              case 8:
                pos = 9;
                break;
              case 9:
                pos = 11;
                break;
              case 11:
                pos = 12;
                break;
              case 12:
                pos = 14;
                break;
              case 14:
                pos = 15;
                break;
          }
          break;
       case BT_UP:
          val[pos]++;
          break;
       case BT_DOWN:
          val[pos]--;
          break;
       }
  
      if(val[pos] < '0')
        val[pos] = '0';
      else if (val[pos] > '9')
        val[pos] = '9'; 
        
      lcd.setCursor(pos, 1);
      lcd.print(val[pos]);
      lcd.setCursor(pos, 1);
    } while((pressed_bt = read_button_blocking()) != BT_SET);
    day = (val[0] - '0')*10+val[1]-'0';
    month = (val[3]-'0')*10+val[4]-'0';
    year = (val[8]-'0')*10+val[9]-'0';
    hour = (val[11]-'0')*10+val[12]-'0';
    min = (val[14]-'0')*10+val[15]-'0';
    Serial.print("day:");
    Serial.println(day);
    Serial.print("month:");
    Serial.println(month);
    Serial.print("year:");
    Serial.println(year);
    Serial.print("hour:");
    Serial.println(hour);
    Serial.print("min:");
    Serial.println(min);

    if(min >= 0 && min < 60
      && hour >= 0 && hour < 24
      && month >= 1 && month <= 12
      && year >= 0 && year <= 99 
      && day >= 0 && day <= dayspermonth[month-1])
              ok = 1;
  } while(!ok);  
  RTC.adjust(DateTime(year, month, day, hour, min, 0));
}

/*
** setting a entry in the menu
*/
void set_function(byte lnb, byte wpower/*, byte wtemp*/)
{
  int place, eelocate;
  int pressed_bt = -1;
  int pos = 0, v;
  char val[16];
  byte h1, m1, h2, m2, power /*temp*/;
  int i;
  int ok = 0;
  place = lnb - 1;
  eelocate = 2+place*5;

  Serial.print("do set light---------------- Number: ");
  Serial.print(lnb);
  Serial.print(" --- with power: ");
  Serial.print(wpower);
  Serial.println("---");
  
  // make sure we are up tu date from EEPROM
  read_eeprom(place);
  h1 = ti[place].h1;   
  m1 = ti[place].m1;   
  h2 = ti[place].h2;   
  m2 = ti[place].m2;   
  power = ti[place].power;
  //temp = ti[place].temp;

  /*
  ** 0123456789012345
  ** 0         1
  ** HH:MM HH:MM XX
  */
  
  val[0] = h1/10+'0';
  val[1] = h1%10+'0';
  val[2] = ':';
  val[3] = m1/10+'0';
  val[4] = m1%10+'0';
  val[5] = ' ';
  val[6] = h2/10+'0';
  val[7] = h2%10+'0';
  val[8] = ':';
  val[9] = m2/10+'0';
  val[10] = m2%10+'0';
  val[11] = ' ';
  val[12] = (wpower) ? power/10+'0' : ' ';
  val[13] = (wpower) ? power%10+'0' : ' ';
  val[14] = ' ';
  //val[14] =  temp/10+'0';
  val[15] = ' ';
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.write("Start Stop ");
  if(wpower)
    lcd.write(" TMP");

  lcd.setCursor(0, 1);
  for(i = 0; i < 16; i++)
    lcd.print(val[i]);

  do {
    do {
      Serial.println("not set button");
      switch(pressed_bt) {
        case BT_LEFT:
          switch(pos) {
              case 1:
                pos = 0;
                break;
              case 3:
                pos = 1;
                break;
              case 4:
                pos = 3;
                break;
              case 6:
                pos = 4;
                break;
              case 7:
                pos = 6;
                break;
              case 9:
                pos = 7;
                break;
              case 10:
                pos = 9;
                break;
              case 12:
                pos = 10;
                break;
              case 13:
                pos = 12;
                break;
            //   case 15:
            //     pos = 13;
            //     break;
          }
          break;
        case BT_RIGHT:
          switch(pos) {
              case 0:
                pos = 1;
                break;
              case 1:
                pos = 3;
                break;
              case 3:
                pos = 4;
                break;
              case 4:
                pos = 6;
                break;
              case 6:
                pos = 7;
                break;
              case 7:
                pos = 9;
                break;
              case 9:
                pos = 10;
                break;
              case 10:
                pos = (wpower) ? 12 : 10;
                break;
              case 12:
                pos = (wpower) ? 13 : 10;
                break;
             // case 13:
             //   pos = 15;
             //   break;
          }
          break;
       case BT_UP:
          val[pos]++;
          break;
       case BT_DOWN:
          val[pos]--;
          break;
       }
  
      if(val[pos] < '0')
        val[pos] = '0';
      else if (val[pos] > '9')
        val[pos] = '9'; 
        
      lcd.setCursor(pos, 1);
      lcd.print(val[pos]);
      lcd.setCursor(pos, 1);
    } while((pressed_bt = read_button_blocking()) != BT_SET);
    h1 = (val[0]-'0')*10+val[1]-'0';
    m1 = (val[3]-'0')*10+val[4]-'0';
    h2 = (val[6]-'0')*10+val[7]-'0';
    m2 = (val[9]-'0')*10+val[10]-'0';
    power = (wpower) ? (val[12]-'0')*10+val[13]-'0' : 0;
    //temp = wtemp;

    if(h1 >= 0 && h1 < 24
      && m1 >= 0 && m1 < 60
      && h2 >= 0 && h2 < 24
      && m2 >= 0 && m2 < 60
      && power >= 0 && power <= 99
      //&& temp >= 0 && temp <= 99
      )
              ok = 1;
  } while(!ok);  
  ti[place].h1 = h1;   
  ti[place].m1 = m1;   
  ti[place].h2 = h2;   
  ti[place].m2 = m2;   
  ti[place].power = power;
  //ti[place].temp = temp;  

  EEPROM.write(eelocate++, h1); // H1  
  EEPROM.write(eelocate++, m1); // M1  
  EEPROM.write(eelocate++, h2); // H2  
  EEPROM.write(eelocate++, m2); // M2  
  EEPROM.write(eelocate++, power); // P1
  //EEPROM.write(eelocate++, temp);  
}

// reads data from EEPROM
void read_eeprom(byte place)
{
  int eelocate;
  eelocate = 2+place*5;
  ti[place].h1 = EEPROM.read(eelocate++);   
  ti[place].m1 = EEPROM.read(eelocate++);   
  ti[place].h2 = EEPROM.read(eelocate++);   
  ti[place].m2 = EEPROM.read(eelocate++);   
  ti[place].power = EEPROM.read(eelocate++);
  //ti[place].temp = EEPROM.read(eelocate++);  

}

// this displays the data on the screen: this function has to be rewritten and the call also. Do not need to redisplay everithing each second
void display_data()
{
  //lcd.clear();
  //lcd.setCursor(0,0);
  //lcd.print("25/08/14");
  //lcd.print(" 23:13");
  //lcd.print(ti[1].power);
  
  // Prints RTC Time on RTC
  now = RTC.now();
  
//  Serial.println("display data");

  // clean up the screen before printing
  lcd.clear();
  // set the cursor to column 0, line 0     
  lcd.setCursor(0, 0);

  // print date
  print2dec(now.day());
  lcd.print('/');
  print2dec(now.month());
  lcd.print('/');
  lcd.print(now.year());
  lcd.print(' ');
  print2dec(now.hour());
  lcd.print(':');
  print2dec(now.minute());
  lcd.print(':');
  print2dec(now.second());
  // move the cursor to the second line
  //lcd.setCursor(0, 1);
  // Print time
  
  
  //lcd.print(' ');
  // Prints statuses
  for(byte i = 0; i < NBSETS; i++) {
    display_out(i);
  }
  
  
  // displays temperature
  lcd.setCursor(0,1);
  int temperature = dht.getTemperature();
  int humidity = dht.getHumidity();
  // test led 
  //int deltaT = temperature - ti[0].power;
  //if (deltaT >= 1) {
  //  digitalWrite(6, HIGH);
  //} else {
  //  digitalWrite(6, LOW);
  //}

  //int deltaU = humidity - ti[1].power;
  //if (deltaU <= 1) {
  //  digitalWrite(7, HIGH);
  //} else {
  //  digitalWrite(7, LOW);
  //}
  
  lcd.print("T:");
  lcd.print(temperature-6);
  lcd.print(" U:");
  lcd.print(humidity+20);
  // Now prints on LCD
  /*
  if(full) {
    lcd.print((int)temperature);
    lcd.print('.');
    lcd.print((int)((temperature+0.05-(int)temperature)*10.0));
  }
  else {
    lcd.print(index); lcd.print("Avr");
  }
  */
}

void display_out(byte i)
{
  lcd.setCursor(10+i, 1);
  switch(out_m[i]) {
    case OFF:
      lcd.print('0');
      break;
    case AUTO:
      lcd.print('A');
      break;
    case ON:
      lcd.print('1');
      break;
    /*case MAX:
      lcd.print('M');
      break;
     */
  }        
}

void print2dec(int nb) { //this adds a 0 before single digit numbers
  if (nb >= 0 && nb < 10) {
    lcd.write('0');
  }
  lcd.print(nb);
}






