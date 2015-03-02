
  /*

   Madrenatura by Alessandro Giuliano 8/2014
   Based on Aquarium controller by (C) 5/2014 Flavius Bindea
   Other contributor: Alessandro Savoini

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
  #include <DataCoder.h>
  #include <VirtualWire.h>
  const int rx_pin = A0;
  const int led_pin = 13;
  const int baudRate = 800;


 // #include <LiquidCrystal.h>
  // include le note musicali
   //#include "pitches.h"
#include <LiquidCrystal_I2C.h>


float temperature;
float humidity;

// used for RTC
  const int dayspermonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  RTC_DS1307 RTC;
  DateTime now;

  // initialize the library with the numbers of the interface pins
  LiquidCrystal_I2C lcd(0x27,16,2);

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

  byte termometro[8] = //icon for termometer
{
    B00100,
    B01010,
    B01010,
    B01110,
    B01110,
    B11111,
    B11111,
    B01110
};

byte goccia[8] = //icon for water droplet
{
    B00100,
    B00100,
    B01010,
    B01010,
    B10001,
    B10001,
    B10001,
    B01110,
};


  // define led
  const int ledVentola = 2;
  const int ledSerpentina = 3;
  const int ledUmidita = 4;
  const int ledLuci = 5;

  // For buttons
  int bstate = 1024, blast = 1024;  // button state and button last state

  // joystick
  #define THRESHOLD_LOW 350
  #define THRESHOLD_HIGH 650

  const byte Pin_VRx = A1;    // Pin inteso qui come verticale
  const byte Pin_VRy = A2;    // Pin inteso qui come orizzontale
  const byte Pin_SW  = 6;    // Pin del bottone sul Joystick

  int x_position;
  int y_position;
  char Prec_x_direction = '\0'; // lettura orizzontale nel precedente loop
  char Prec_y_direction = '\0'; // lettura verticale nel precedente loop
  char Prec_b_state = '\0';     // stato del bottone nel precedente loop
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
  const byte cols = 20, lines = 4;

  // menu of status
  const int menumin = 0;
  const int menumax = 3;

  char* menu_entry[] = {
    "1. Temperatura ",
    "2. Umidita'    ",
    "3. Luci        ",
    "4. Set Data/Ora",
    //"5. notte       "
    //"6. Menu vuoto 6 "
  };

  // status of programm
  #define ST_DISPLAY 0
  #define ST_MENU 1
  int status = ST_DISPLAY;

  // function prototypes
  void set_function(byte lnb, byte wpower=1);

  // Define the devices
  #define ventola 10
  #define serpentina 12
  #define umidita 9
  #define luci 8

  struct AQTIME {
    byte h1;
    byte m1;
    byte h2;
    byte m2;
    byte power; // si riferisce alla valore temperatura o umidità di riferimento impostato
  };

  // number of setups in memory
  #define NBSETS 4
  AQTIME ti[NBSETS];
  byte out[NBSETS];

  // statuses of outputs
  #define OFF 0
  #define AUTO 1
  #define ON 2
  byte out_m[NBSETS];

  // EEPROM signature for aquarium: they are stored in 0 and 1
  const byte AQ_SIG1 = 45, AQ_SIG2 = 899;

#include "pitches.h"

// notes in the melody:
int melody[] = {  NOTE_C4, NOTE_G3,NOTE_G3, NOTE_A3, NOTE_G3,0, NOTE_B3, NOTE_C4};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4,4,4,4,4 };

  // Initial setup
  void setup()
  {
    lcd.backlight();
    Serial.begin(9600);
    Serial.println("Welcome to Madrenatura");

    // Configures RTC
    Wire.begin(); // initalise I2C interface

    if (! RTC.isrunning()) {
      Serial.println("RTC is NOT running!");
      // following line sets the RTC to the date & time this sketch was compiled
      RTC.adjust(DateTime(__DATE__, __TIME__));
    }
    SetupRFDataRxnLink(rx_pin, baudRate);
    // If you want to set the aref to something other than 5v
    //analogReference(EXTERNAL);

    // Configures display

    lcd.begin(cols, lines);
    lcd.createChar(1, up);
    lcd.createChar(2, down);
    lcd.createChar(3, termometro);
    lcd.createChar(4, goccia);
    // Print a message to the LCD.
    lcd.setCursor(0,0);
    lcd.print("Madrenatura");
    lcd.setCursor(0, 1);
    lcd.print("Made in Italy");
    delay(500);

    // trys to read EEPROM
    if(AQ_SIG1 != EEPROM.read(0) || AQ_SIG2 != EEPROM.read(1)) {
      lcd.print(" NOT SET");
      delay(2000);
      EEPROM.write(0, AQ_SIG1);
      EEPROM.write(1, AQ_SIG2);
      for(int i = 2; i < 2+NBSETS*5; i++) {
        EEPROM.write(i, 0);
      }
    } else {
      // reads the EEPROM setup
      read_eeprom(0);
      read_eeprom(1);
      read_eeprom(2);
      read_eeprom(3);
    }

    // setout leds
    pinMode(ledVentola, OUTPUT);
    pinMode(ledSerpentina, OUTPUT);
    pinMode(ledUmidita, OUTPUT);
    pinMode(ledLuci, OUTPUT);
    pinMode(umidita, OUTPUT);
    pinMode(luci, OUTPUT);
    pinMode(ventola, OUTPUT);
    pinMode(serpentina, OUTPUT);
    pinMode(7, OUTPUT);
    // Set initial state
    digitalWrite(umidita, LOW);
    digitalWrite(luci, LOW);
    digitalWrite(ventola, LOW);
    digitalWrite(serpentina, LOW);

    digitalWrite(ledVentola, LOW);
    digitalWrite(ledUmidita, LOW);
    digitalWrite(ledLuci, LOW);
    digitalWrite(ledSerpentina, LOW);


    out[0] = ventola;
    out[1] = serpentina;
    out[2] = umidita;
    out[3] = luci;


    for(int i = 0; i < NBSETS; i++) {
      out_m[i] = AUTO;
    }
    delay(50);

    // joystick
    pinMode(Pin_SW, INPUT);      // Inizializzo il pin del pulsante del JOYSTICK
    digitalWrite(Pin_SW,HIGH);   // Setto la resistenza di pull-up
    //wireless begin
     // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

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

    lcd.clear();

  }

  /****************************************************************************************************************
  ******************                              MAIN LOOP                       *********************************
  ****************************************************************************************************************/
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
        // save lasted display millis
        previousDisplayMillis = currentMillis;
        // display the data on the screen
        display_data();
        display_sensor();
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
    receive();
     // small delay
     delay(50);
  }

  /********************************************************************************************************
  **                       END MAIN LOOP                                   ********************************
  ********************************************************************************************************/

  /***************************************************************
  **                  switch the menu status                    **
  ***************************************************************/

  void chg_status()
  {
    if(status == ST_DISPLAY) {
      lcd.blink();
      status = ST_MENU;
    } else {
      lcd.noBlink();
      status = ST_DISPLAY;
    }
  }

  /***************************************************************
  **                   switch out put mode                      **
  ***************************************************************/

  void switch_out(byte n)
  {
    switch(out_m[n]) {
      case ON:
        out_m[n] = AUTO;
        break;
      case AUTO:
        out_m[n] = OFF;
        break;
      case OFF:
        out_m[n] = ON;
        break;
    }
    display_out(n);
  }

  /******************************************************************
  **            return button status if it has changed             **
  ******************************************************************/

  int read_button()
  {
    char x_direction = '\0';
    char y_direction = '\0';
    char b_state = '\0';

    // Pin_VRx è abbinato con y e viceversa per
    // via dell'interpretazione diversa che do ai
    // movimenti del joystick
    x_position = analogRead(Pin_VRy);
    y_position = analogRead(Pin_VRx);
    b_state=digitalRead(Pin_SW);  // Leggo lo stato del pulsante del joystick

    // Stabilisco la direzione e stato bottone
    if (x_position > THRESHOLD_HIGH)
      x_direction = 'R'; // Destra
    else if (x_position < THRESHOLD_LOW)
      x_direction = 'L'; // Sinistra
    else
      x_direction = 'C'; // Riposo
    if (y_position > THRESHOLD_HIGH)
      y_direction = 'U'; // Alto
    else if (y_position < THRESHOLD_LOW)
      y_direction = 'D'; // Basso
    else
      y_direction = 'C'; // Riposo
    if (digitalRead(Pin_SW)==LOW)
       b_state='P'; // è premuto
    else
       b_state='-';

    if (Prec_x_direction != x_direction || Prec_y_direction != y_direction || Prec_b_state != b_state)
        //Serial.println(String(x_direction)+String(y_direction)+String(b_state));
        Prec_x_direction = x_direction;
        Prec_y_direction = y_direction;
        Prec_b_state=b_state;
        delay(50);

    int button;

    blast = bstate;

    if (x_direction == 'R')
      bstate = 2;
    else if (x_direction == 'L')//(button >= button2min && button <= button2max)
      bstate = 3;
    else if (y_direction == 'U')//(button >= button3min && button <= button3max)
      bstate = 4;
    else if (y_direction == 'D')//(button >= button4min && button <= button4max)
      bstate = 5;
    else if (b_state=='P')//(button >= button5min && button <= button5max)
      bstate = 1;
    else if (b_state=='-')//(button >= buttonNONE)
      bstate = 0;
    else
      bstate = 99; // we should never arrive here

    //if(bstate == 99) {
    //  Serial.print("ERROR: "); Serial.println(button);
    //}

    if (blast != bstate) {
      // state has changed
      if(bstate >=1 && bstate <= 5) {
        Serial.print("BUTTON: "); Serial.println(bstate);
        return(bstate);
      }
    }
    return(0);
  }

  /************************************************************
  **                     read blocking                       **
  ************************************************************/

  int read_button_blocking()
  {
    int i;

    while((i = read_button()) == 0)
      delay(50);
    return i;
  }
 void receive(){
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;
  union RFData inDataSeq;//To store incoming data
  float inArray[2];//To store decoded information
  if(RFLinkDataAvailable(buf, &buflen))
  {
        for(int i =0; i< buflen; i++)
        {
          inDataSeq.s[i] = buf[i];

        }
        DecodeRFData(inArray, inDataSeq);
        int time = millis() / 1000;
        Serial.print(time);
        Serial.print(" ");
        Serial.print("Temperature: ");
        Serial.print(inArray[0]);
        Serial.print(" ° Humidity: ");
        Serial.println(inArray[1]);
        temperature = inArray[0];
        humidity = inArray[1];
      }
 }
  /*************************************************************
  **                does interval calculations                **
  *************************************************************/

  void calculations()
  {
    int h, m;

    // read the date
    now = RTC.now();
    h = now.hour();
    m = now.minute();
    receive();
    //temperature = inArray[0];
    //humidity = inArray[1];
    // setting the status of the outputs
    for(int li = 0; li < 4; li++) {

      byte out_s;

      if(out_m[li] == OFF) {
        out_s = OFF;
          if (li < 1 ) {
            digitalWrite(ledVentola,LOW);
          } else if (li < 2) {
            digitalWrite(ledSerpentina,LOW);
          } else if (li < 3) {
            digitalWrite(ledUmidita,LOW);
          } else if (li < 4) {
            digitalWrite(ledLuci,LOW);
          }

      } else if (out_m[li] == ON) {
        out_s = ON;
        if (li < 1 ) {
            digitalWrite(ledVentola,HIGH);
          } else if (li < 2) {
            digitalWrite(ledSerpentina,HIGH);
          } else if (li < 3) {
            digitalWrite(ledUmidita,HIGH);
          } else if (li < 4) {
            digitalWrite(ledLuci,HIGH);
          }

      } else if (out_m[li] == AUTO) {
        // programma 1 temperatura per ventola
        if (li < 1) {

          if (ti[0].power < temperature) {
            unsigned long oraInSecondi = now.hour() * 3600L;
            unsigned long minutiInSecondi = now.minute() * 60L;
            long oraInizio = ti[0].h1;
            long minutiInizio = ti[0].m1;
            long oraFine = ti[0].h2;
            long minutiFine = ti[0].m2;
            unsigned long secondiInizio = 0;
            unsigned long secondiFine = 0;
            unsigned long secondiAttuali = 0;
            secondiAttuali = oraInSecondi + minutiInSecondi;
            secondiInizio = (oraInizio * 3600 + minutiInizio * 60);
            secondiFine = (oraFine * 3600 + minutiFine * 60);

            if (secondiInizio < secondiFine) { // caso normale 8:00 18:00
              if (secondiInizio < secondiAttuali && secondiAttuali < secondiFine ) {
                out_s=ON;
                digitalWrite(ledVentola,HIGH);
              } else {
                out_s=OFF;
                digitalWrite(ledVentola,LOW);
              }
            } else {                  // caso 18:00 8:00
              if (secondiInizio < secondiAttuali || secondiAttuali < secondiFine) {
                out_s=ON;
                digitalWrite(ledVentola,HIGH);
              } else {
                out_s=OFF;
                digitalWrite(ledVentola,LOW);
              }
            }

          } else {
            out_s = OFF;
            digitalWrite(ledVentola,LOW);
          }
        // programma 2   temperatura per serpentina
       } else if (li < 2) {

          if (ti[0].power > temperature) {
           unsigned long oraInSecondi = now.hour() * 3600L;
           unsigned long minutiInSecondi = now.minute() * 60L;
           long oraInizio = ti[0].h1;
           long minutiInizio = ti[0].m1;
           long oraFine = ti[0].h2;
           long minutiFine = ti[0].m2;
           unsigned long secondiInizio = 0;
           unsigned long secondiFine = 0;
           unsigned long secondiAttuali = 0;
           secondiAttuali = oraInSecondi + minutiInSecondi;
           secondiInizio = (oraInizio * 3600 + minutiInizio * 60);
           secondiFine = (oraFine * 3600 + minutiFine * 60);

          if (secondiInizio < secondiFine) { // caso normale 8:00 18:00
            if (secondiInizio < secondiAttuali && secondiAttuali < secondiFine ) {
              out_s=ON;
              digitalWrite(ledSerpentina,HIGH);
            } else {
              out_s=OFF;
              digitalWrite(ledSerpentina,LOW);
            }
          } else {                  // caso 18:00 8:00
            if (secondiInizio < secondiAttuali || secondiAttuali < secondiFine) {
              out_s=ON;
              digitalWrite(ledSerpentina,HIGH);
            } else {
              out_s=OFF;
              digitalWrite(ledSerpentina,LOW);
            }
          }

        } else {
            out_s = OFF;
            digitalWrite(ledSerpentina,LOW);
        }
        // programma 3 umidità
       } else if (li < 3) {

        if (ti[1].power > humidity) {
           unsigned long oraInSecondi = now.hour() * 3600L;
           unsigned long minutiInSecondi = now.minute() * 60L;
           long oraInizio = ti[1].h1;
           long minutiInizio = ti[1].m1;
           long oraFine = ti[1].h2;
           long minutiFine = ti[1].m2;
           unsigned long secondiInizio = 0;
           unsigned long secondiFine = 0;
           unsigned long secondiAttuali = 0;
           secondiAttuali = oraInSecondi + minutiInSecondi;
           secondiInizio = (oraInizio * 3600 + minutiInizio * 60);
           secondiFine = (oraFine * 3600 + minutiFine * 60);

          if (secondiInizio < secondiFine) { // caso normale 8:00 18:00
            if (secondiInizio < secondiAttuali && secondiAttuali < secondiFine ) {
              out_s=ON;
              digitalWrite(ledUmidita,HIGH);
            } else {
              out_s=OFF;
              digitalWrite(ledUmidita,LOW);
            }
          } else {                  // caso 18:00 8:00
            if (secondiInizio < secondiAttuali || secondiAttuali < secondiFine) {
              out_s=ON;
              digitalWrite(ledUmidita,HIGH);
            } else {
              out_s=OFF;
              digitalWrite(ledUmidita,LOW);
            }
          }

        } else {
          out_s = OFF;
          digitalWrite(ledUmidita,LOW);
        }
        // programma 4 luci
       } else if (li < 4) {
           unsigned long oraInSecondi = now.hour() * 3600L;
           unsigned long minutiInSecondi = now.minute() * 60L;
           long oraInizio = ti[2].h1;
           long minutiInizio = ti[2].m1;
           long oraFine = ti[2].h2;
           long minutiFine = ti[2].m2;
           unsigned long secondiInizio = 0;
           unsigned long secondiFine = 0;
           unsigned long secondiAttuali = 0;
           secondiAttuali = oraInSecondi + minutiInSecondi;
           secondiInizio = (oraInizio * 3600 + minutiInizio * 60);
           secondiFine = (oraFine * 3600 + minutiFine * 60);
          if (secondiInizio < secondiFine) { // caso normale 8:00 18:00
            if (secondiInizio < secondiAttuali && secondiAttuali < secondiFine ) {
              out_s=ON;
              digitalWrite(ledLuci,HIGH);
            } else {
              out_s=OFF;
              digitalWrite(ledLuci,LOW);
            }
          } else {                  // caso 18:00 8:00
            if (secondiInizio < secondiAttuali || secondiAttuali < secondiFine) {
              out_s=ON;
              digitalWrite(ledLuci,HIGH);
            } else {
              out_s=OFF;
              digitalWrite(ledLuci,LOW);
            }
          }
        }
      }
      // apre e chiude effettivamente i rele
        if(out_s == OFF)
          digitalWrite(out[li], HIGH);
        else
          digitalWrite(out[li], LOW);
    }

  }

  /**************************************************************
  **                     does the menu                         **
  **************************************************************/

  void do_menu()
  {
    int pressed_bt = -1;
    int menuline = 0;

    Serial.println("do menu---------------------------------");

    start_menu();

    do {

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
      lcd.print(menu_entry[menuline]);
      lcd.print("\076");
      lcd.setCursor(15, 1);
    } while ((pressed_bt = read_button_blocking()) != BT_SET);

    //Serial.println("SET button pressed")
   chg_status();
  }


  void start_menu() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Menu:  ");
    lcd.print((char)1);
    lcd.print((char)2);
    lcd.print(' ');
    lcd.print((char)126);
    lcd.print("OK");
  }

  void do_menu_entry(int en) {
    Serial.print("Do menu entry:");
    Serial.println(en);

    switch(en) {
       case 0:
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
       case 4:
         set_function(4);
         break;
    }
  }

  /***************************************************
  **            Menu entry to setup the time        **
  ***************************************************/

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
    lcd.print("Time: use");
    lcd.print(char(1));
    lcd.print(char(2));
    lcd.print(char(127));
    lcd.print(char(126));
    lcd.print("SET");

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
          val[pos] = '9';
        else if (val[pos] > '9')
          val[pos] = '0';

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
    start_menu();
  }

  /************************************************************
  **           setting a entry in the menu                   **
  ************************************************************/

  void set_function(byte lnb, byte wpower)
  {
    int place, eelocate;
    int pressed_bt = -1;
    int pos = 0, v;
    char val[16];
    byte h1, m1, h2, m2, power;
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
    val[15] = ' ';

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Start Stop ");
    if(wpower)
      lcd.print(" VAL");

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
          val[pos] = '9';
        else if (val[pos] > '9')
          val[pos] = '0';

        lcd.setCursor(pos, 1);
        lcd.print(val[pos]);
        lcd.setCursor(pos, 1);
      } while((pressed_bt = read_button_blocking()) != BT_SET);
      h1 = (val[0]-'0')*10+val[1]-'0';
      m1 = (val[3]-'0')*10+val[4]-'0';
      h2 = (val[6]-'0')*10+val[7]-'0';
      m2 = (val[9]-'0')*10+val[10]-'0';
      power = (wpower) ? (val[12]-'0')*10+val[13]-'0' : 0;

      if(h1 >= 0 && h1 < 24
        && m1 >= 0 && m1 < 60
        && h2 >= 0 && h2 < 24
        && m2 >= 0 && m2 < 60
        && power >= 0 && power <= 99
        )
                ok = 1;
    } while(!ok);
    ti[place].h1 = h1;
    ti[place].m1 = m1;
    ti[place].h2 = h2;
    ti[place].m2 = m2;
    ti[place].power = power;

    EEPROM.write(eelocate++, h1); // H1
    EEPROM.write(eelocate++, m1); // M1
    EEPROM.write(eelocate++, h2); // H2
    EEPROM.write(eelocate++, m2); // M2
    EEPROM.write(eelocate++, power); // P1
    start_menu();
  }

  /*********************************************************
  **             reads data from EEPROM                   **
  *********************************************************/

  void read_eeprom(byte place)
  {
    int eelocate;
    eelocate = 2+place*5;
    ti[place].h1 = EEPROM.read(eelocate++);
    ti[place].m1 = EEPROM.read(eelocate++);
    ti[place].h2 = EEPROM.read(eelocate++);
    ti[place].m2 = EEPROM.read(eelocate++);
    ti[place].power = EEPROM.read(eelocate++);
  }

  /************************************************************
  **      this displays the data on the screen               **
  ************************************************************/

  void display_data()
  {
    // Prints RTC Time on RTC
    now = RTC.now();
    lcd.setCursor(0, 0);
    print2dec(now.day());
    lcd.print('/');
    print2dec(now.month());
    lcd.print('/');
    lcd.print(now.year());
    lcd.print(' ');
    print2dec(now.hour());
    lcd.print(':');
    print2dec(now.minute());
    //lcd.print(':');
    //print2dec(now.second());
    // Prints statuses
    for(byte i = 0; i < NBSETS; i++) {
      display_out(i);
    }
  }

  /********************************************************
  **                display temp and humidity            **
  ********************************************************/

  void display_sensor()
  {

        receive();
        lcd.setCursor(0,1);
        lcd.print((char)3);
        lcd.print(" ");
        lcd.print(temperature);
        lcd.print((char)223);
        lcd.print(" ");
        lcd.print((char)4);
        lcd.print(" ");
        lcd.print(humidity);
        lcd.print("%");
        lcd.print(" ");
      }


  /**********************************************************
  **              display modalità dispositivi             **
  **********************************************************/

  void display_out(byte i)
  {
    byte lampadina[8] = { B01110, B10001, B10101, B10101, B11011, B01110, B01110, B01110 };
    lcd.createChar(5, lampadina);
    byte fiamma[8] = { B00100, B00110, B01011, B01011, B11110, B11110, B01100, B00100 };
    lcd.createChar(6, fiamma);
    byte fan[8] = { B00111, B10110, B10100, B01010, B01010, B00101, B01101, B11100 };
    lcd.createChar(7, fan);
    lcd.setCursor(1,2);
    lcd.print((char)7);
    lcd.setCursor(6,2);
    lcd.print((char)6);
    lcd.setCursor(11,2);
    lcd.print((char)4);
    lcd.setCursor(16,2);
    lcd.print((char)5);

    char autochar[]= "Auto";
    char offchar[]="Off ";
    char onchar[]="On ";
    int x;
    if (i == 0){
      x=0;
    } else if(i==1){
      x=4;
    } else if(i==2){
      x=8;
    } else if(i==3){
      x=12;
    }
    lcd.setCursor(x+i, 3);
    switch(out_m[i]) {
      case OFF:
        lcd.print(offchar);
        break;
      case AUTO:
        lcd.print(autochar);
        break;
      case ON:
        lcd.print(onchar);
        break;
    }
  }
  /***************************************************
  **   this adds a 0 before single digit numbers    **
  ***************************************************/

  void print2dec(int nb) {
    if (nb >= 0 && nb < 10) {
      lcd.print('0');
    }
    lcd.print(nb);
  }

