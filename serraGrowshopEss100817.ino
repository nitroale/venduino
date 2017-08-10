
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
  #include <RTClib.h> // From: https://github.com/adafruit/RTClib.git 
  #include <EEPROM.h> // Fro read and write EEPROM
  //#include <VirtualWire.h>
  #include <DHT.h>
  #define DHTPIN A0     // what pin we're connected to
  #define DHTTYPE DHT22   // DHT 11
  DHT dht(DHTPIN, DHTTYPE);
  #include <LiquidCrystal_I2C.h>

  byte dayUno;
unsigned long previousEssMillis = 0;
const long intervalEss = 7020000;   
unsigned long currentEssMillis;
int minutoAccensioneEss;
int minutoSpegnimentoEss;
int minutoAttualeEss;

  //float temperatureV;
  //float temperatureS;
  float temperature, humidity;
  int vState = 0;
  int sState = 0;
  int uState = 0;
  float delta = 0;
  float deltaS = 0;
  float deltaU = 0;

  // used for RTC
  const int dayspermonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
  RTC_DS1307 RTC;
  DateTime now;
  //unsigned long giorniTrascorsi;
  //unsigned long impostaTimer;

  // initialize the library with the numbers of the interface pins
  LiquidCrystal_I2C lcd(0x27,20,4);

  // 126: -> 127: <-
  byte up[8] = { B00100,B01110,B10101,B00100,B00100,  B00100,  B00100,};
  byte down[8] = {  B00100,  B00100,  B00100,  B00100,  B10101,  B01110,  B00100,};
  byte termometro[8] ={ B00100, B01010, B01010,  B01110,  B01110,  B11111,  B11111,  B01110};
  byte goccia[8] = {B00100,   B00100,   B01010,   B01010,   B10001,   B10001,   B10001,   B01110,};
  byte lampadina[8] = { B01110, B10001, B10101, B10101, B11011, B01110, B01110, B01110 };
  byte fiamma[8] = { B00100, B00110, B01011, B01011, B11110, B11110, B01100, B00100 };
  byte fan[8] = { B00111, B10110, B10100, B01010, B01010, B00101, B01101, B11100 };

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
  //unsigned long previousDisplayMillis = 0;
  //unsigned long displayInterval = 1000;
  // For looping calculation by interval
  //unsigned long previousCalculationMillis = 0;
  //unsigned long calculationInterval = 250;

  unsigned long intervallo;
  unsigned long dhtMillis = 5000;
  unsigned long previousMillisDht = 0;
  unsigned long intervalloButton;
  int priorita;
  
  // menu of status
  const int menumin = 0;
  const int menumax = 4;

  char* menu_entry[] = {
    "1. Temperatura ",
    "2. Umidita'    ",
    "3. Luci        ",
    "4. Essicazione ",
    "5. Set Data/Ora"
    //"5. Essicazione "
    //"6. Menu vuoto 6 "
  };

  // status of programm
  #define ST_DISPLAY 0
  #define ST_MENU 1
  #define ST_ESSICAZIONE 2
  int status = ST_DISPLAY;

  // function prototypes
  void set_function(byte lnb, byte wpower=1, byte wpowerN=1);

  // Define the devices
  #define ventola 13
  #define serpentina 12
  #define umidita 11
  #define luci 10
  //umidità terreno 9
  //co2 8

  struct AQTIME {
    byte h1;
    byte m1;
    byte h2;
    byte m2;
    byte power; // si riferisce alla valore temperatura o umidità di riferimento impostato
    byte powerN;
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
  

  int programma;

  // Initial setup
  void setup()
  { 
    
    lcd.backlight();
    

    // Configures RTC
    Wire.begin(); // initalise I2C interface

    if (! RTC.isrunning()) {
      //Serial.println("RTC is NOT running!");
      // following line sets the RTC to the date & time this sketch was compiled
      RTC.adjust(DateTime(__DATE__, __TIME__));
    }

    // Configures display
    lcd.begin(20, 4);
    lcd.createChar(1, up);
    lcd.createChar(2, down);
    lcd.createChar(3, termometro);
    lcd.createChar(4, goccia);
    // Print a message to the LCD.
    lcd.setCursor(0,0);
    lcd.print(F("Smebu"));
    lcd.setCursor(0, 1);
    lcd.print(F("Growshop 2.0"));
    delay(5000);

    // trys to read EEPROM
    if(AQ_SIG1 != EEPROM.read(0) || AQ_SIG2 != EEPROM.read(1)) {
      lcd.print(F(" NOT SET"));
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
      //for (int i=0; i < 4; i++)
        //Serial.println(EEPROM.read(i));
    }

    dayUno = EEPROM.read(60);
    if (dayUno == 0) {
     Serial.println("modo auto");
    } else {
      Serial.println("modo essicazione");
    }
    // setout pin
    pinMode(umidita, OUTPUT);
    pinMode(luci, OUTPUT);
    pinMode(ventola, OUTPUT);
    pinMode(serpentina, OUTPUT);
    // Set initial state
    digitalWrite(umidita, LOW);
    digitalWrite(luci, LOW);
    digitalWrite(ventola, LOW);
    digitalWrite(serpentina, LOW);

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
    
    dht.begin();
    lcd.clear();
    
    intervallo = millis() + 200;
   // intervalloButton = millis() + 100;
    Serial.begin(9600);
    Serial.println("avvio...");
  }

  /****************************************************************************************************************
  ******************                              MAIN LOOP                       *********************************
  ****************************************************************************************************************/
  void loop()
  {
   // int programma = EEPROM.read(programma);
    unsigned long currentMillis = millis();
    /*if((unsigned long)(millis() - intervalloButton) >=0 ){
      intervalloButton += 200;
        button();
    }*/
    button();

    if (dayUno == 0) {

        if(currentMillis - previousMillisDht > dhtMillis ){
          previousMillisDht = currentMillis;
            calculations();
            display_sensor();
        }
        if(status == ST_DISPLAY) {
          if((unsigned long)(millis() - intervallo) >=0 ){
            intervallo += 100;
            display_data();
            //programma = EEPROM.read(50);
            //Serial.println(programma);
          }
        }
    } else {
      display_sensor();
      display_day();
      /*programma = EEPROM.read(50);
      //Serial.println(programma);
      Serial.print("impostaTimer: ");
      Serial.println(impostaTimer);
      Serial.print("giorniTrascorsi: ");
      Serial.println(giorniTrascorsi);*/
      timerEssicazione();
      calculationsEss();
    }
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
        Prec_x_direction = x_direction;
        Prec_y_direction = y_direction;
        Prec_b_state=b_state;
        delay(50);

    //int button;

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
        //Serial.print("BUTTON: "); 
        //Serial.println(bstate);
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

  /*************************************************************
  **                does interval calculations                **
  *************************************************************/

  void calculations()
  {
    //int numeroProgramma = EEPROM.read(50);
    int h, m;
    byte out_s = AUTO;
    // read the date
    now = RTC.now();
    h = now.hour();
    m = now.minute();
    //float temperatureV = dht.readTemperature();
    //float temperatureS = dht.readTemperature();
    //float humidityR = dht.readHumidity();
    float temperature = dht.readTemperature();
    Serial.println(temperature);
    float humidity = dht.readHumidity();



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
    boolean giorno;
    
      if (secondiInizio < secondiFine) {
        giorno = ((secondiAttuali < secondiFine) && (secondiInizio < secondiAttuali));
      } else {
         giorno = (((secondiAttuali < 86400) && (secondiInizio < secondiAttuali)) || ((secondiAttuali < secondiFine) && (0 <= secondiAttuali)));
      }

    

    for(int li = 0; li < 4; li++) {
      if(out_m[li] == OFF) {
        out_s = OFF;
      } else if (out_m[li] == ON) {
        out_s = ON;
      } else if (out_m[li] == AUTO) {
        // se è giorno
        if (giorno == 1){
            // ventola
            if (li < 1){
              if (vState == 0){
                  delta = 1.3;
                } else {
                  delta = 0;
                }
             // out_s=ON;
              if (now.minute() < 2) {
                  out_s=ON;
                } else if ((ti[0].power + delta) < temperature) {
                //            20                    22          60           50
                  //Serial.println0"Ventole accesa giorno");
                  out_s=ON;
                  vState = 1;
                } else if (ti[0].power > temperature) {
                  //             20          22
                  //Serial.println("Ventole spenta giorno");
                  out_s=OFF;
                  vState = 0;
                } else {
                  out_s=OFF;
                  vState = 0;
                }

            } else if (li < 2){
              if (sState == 0){
                  deltaS = 1.3;
                } else {
                  deltaS = 0;
                }
              // se t impostata è maggiore di quella letta accendi le serpentine
               if (ti[0].power - deltaS > temperature) {
                      //20          0      19
                  //Serial.println("Serpentina accesa giorno");
                out_s=ON;
                sState = 1; 

              } else if (ti[0].power < temperature){
                out_s=OFF;
                sState = 0;
              } else {
                  //Serial.println("Serpentina spenta giorno");
                  out_s=OFF;
                  sState = 0;
                }
            } else if (li < 3){
              if (uState == 0){
                  deltaU = 3;
                } else {
                  deltaU = 0;
                }
              if (now.minute() < 2) {
                  out_s=ON;
              }else if ((ti[1].power + deltaU) > humidity){
                        //    60           0          64
                  //Serial.println("umidita accesa giorno");
                  out_s=OFF;
                  uState = 0;
              } else if (ti[1].power < humidity){
                       //     60            64
                  //Serial.println("umidita accesa giorno");
                  out_s=ON;
                  uState = 1;
              } else {
                  //Serial.println("umidita spenta giorno");
                  out_s=ON;
                  uState = 1;
                }
              /*if (now.minute() < 1) {
                  out_s=OFF;
              } else if ((ti[1].power > humidity) && (ti[0].power >= temperature)){
                  //Serial.println("umidita accesa giorno");
                  out_s=ON;
              } else {
                  //Serial.println("umidita spenta giorno");
                  out_s=OFF;
                }*/
            } else if (li < 4) {
              // è giorno quindi accendo le luci
              //Serial.println("luce accesa giorno");
              out_s=ON;
            }
        } else {
        // se è notte 
          if (li < 1){
            if (vState == 0){
                  delta = 1.3;
                } else {
                  delta = 0;
                }
              if (now.minute() < 2) {
                  out_s=ON;
              } else if ((ti[0].powerN + delta) < temperature) {
                  //Serial.println("Ventole accesa notte");
                out_s=ON;
                vState = 1;
              } else if (ti[0].powerN > temperature) {
                  //Serial.println("Ventole spenta notte");
                  out_s=OFF;
                  vState = 0;
                } else {
                  out_s=OFF;
                  vState = 0;
                }
          } else if (li < 2) {
                if (sState == 0){
                deltaS = 1.3;
              } else {
                deltaS = 0;
              }
              // se t impostata è maggiore di quella letta accendi le serpentine
              if ((ti[0].powerN - deltaS) > temperature) {
                  //Serial.println("Serpentina accesa notte");
                  out_s=ON;
                  sState = 1;
              } else if (ti[0].powerN < temperature){
                  //Serial.println("Serpentina spenta notte");
                  out_s=OFF;
                  sState = 0;
                } else {
                  out_s=OFF;
                  sState = 0;
                }
            } else if (li < 3){
              if (uState == 0){
                  deltaU = 3;
                } else {
                  deltaU = 0;
                }
              if (now.minute() < 2) {
                  out_s=ON;
              }else if ((ti[1].powerN + deltaU) > humidity){
                  //Serial.println("umidita accesa giorno");
                  out_s=OFF;
                  uState = 0;
              } else if (ti[1].powerN < humidity){
                 //     60            64
                //Serial.println("umidita accesa giorno");
                out_s=ON;
                uState = 1;
              } else {
                  //Serial.println("umidita spenta giorno");
                  out_s=ON;
                  uState = 1;
                }
              /*if (now.minute() < 1) {
                  out_s=OFF;
              } else if ((ti[1].powerN > humidity) && (ti[0].powerN >= temperature)) {
                    out_s=ON;
              } else {
                  //Serial.println("umidita spenta notte");
                out_s=OFF;
            }*/
            } else if (li < 4){
                //Serial.println("luce spenta notte");
              out_s=OFF;
            }
        }
      }
      // apre e chiude effettivamente i rele
        if(out_s == OFF)
          digitalWrite(out[li], LOW);
        else
          digitalWrite(out[li], HIGH);
    }

  }

  /**************************************************************
  **                     does the menu                         **
  **************************************************************/

  void do_menu()
  {
    int pressed_bt = -1;
    int menuline = 0;

    //Serial.println("do menu---------------------------------");

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
    lcd.print(F("Menu:  "));
    lcd.print((char)1);
    lcd.print((char)2);
    lcd.print(' ');
    lcd.print((char)126);
    lcd.print(F("OK"));
  }

  void do_menu_entry(int en) {
    //Serial.print("Do menu entry:");
    //Serial.println(en);

    switch(en) {
       case 0:
        set_function(1);
         break;
       case 1:
         set_function(2);
         break;
       case 2:
         set_function(3,0,0);
         break;
       case 3:
         essicazione();
         break;
       case 4:
         set_time();
         break;
    }
  }

  /***************************************************
  **            Menu entry to setup the time        **
  ***************************************************/

  void set_time()
  {
    int pressed_bt = -1;
    int pos = 0;
    char val[16];
    int day, month, year, hour, min;
    int i;
    int ok = 0;

    //Serial.println("do set time---------------------------------");
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
    lcd.print(F("Time: use"));
    lcd.print(char(1));
    lcd.print(char(2));
    lcd.print(char(127));
    lcd.print(char(126));
    lcd.print(F("SET"));

    lcd.setCursor(0, 1);
    for(i = 0; i < 16; i++)
      lcd.print(val[i]);

    do {
      do {
        //Serial.println("not set button");
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
      //Serial.print("day:");
      //Serial.println(day);
      //Serial.print("month:");
      //Serial.println(month);
      //Serial.print("year:");
      //Serial.println(year);
      //Serial.print("hour:");
      //Serial.println(hour);
      //Serial.print("min:");
      //Serial.println(min);

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


  void essicazione(){
    int pressed_bt = -1;

    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Modo Essicazione");
    lcd.setCursor(0,1);
    lcd.print("Sinistra annulla");
    lcd.setCursor(0,2);
    lcd.print("");
    lcd.setCursor(0,3);
    lcd.print("Premi per avviare");

    if((pressed_bt = read_button_blocking()) == BT_LEFT){
      currentEssMillis = 0;
      dayUno = 0;
      EEPROM.write(60, 0);
      lcd.clear();
      start_menu();
   }
    else if ((pressed_bt = read_button_blocking()) == BT_SET){
      //RTC.adjust(DateTime(0, 1, 1, 0, 0, 0));
      dayUno = now.day();
      EEPROM.write(60, dayUno);
      timerEssicazione();
      start_menu();
    }
  }

void timerEssicazione(){

    DateTime now = RTC.now();
    int  dayNow = now.day();
    dayUno = EEPROM.read(60);
    //Serial.print("Giorno di essicazione: ");
    //Serial.println((dayNow - dayUno) + 1);
    currentEssMillis = millis();



      if (currentEssMillis - previousEssMillis >= intervalEss) {
          // save the last time you blinked the LED
          previousEssMillis = currentEssMillis;

          minutoAccensioneEss = now.minute();
          minutoSpegnimentoEss = minutoAccensioneEss + 2;
          minutoAttualeEss = now.minute();
      }

      if ((minutoAttualeEss >= minutoAccensioneEss ) && (minutoSpegnimentoEss >= minutoAttualeEss)){
          digitalWrite(8, HIGH);
          digitalWrite(10, HIGH);
          //ledState = ON;
          //Serial.println("ON");
          /*Serial.print("minutoAttuale: ");
          Serial.println(minutoAttuale);
          Serial.print("minutoAccensione: ");
          Serial.println(minutoAccensione);
          Serial.print("minutoSpegnimento: ");
          Serial.println(minutoSpegnimento); */
          minutoAttualeEss = now.minute();
          priorita = 0;
      } else {
          //digitalWrite(8, LOW);
          //digitalWrite(10, LOW);
         // Serial.println("OFF");
        priorita = 1;
      }

    }

void calculationsEss() {
    byte out_s;
    
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    /*for(int li = 0; li < 4; li++) {
      /*if(out_m[li] == OFF) {
        out_s = OFF;
      } else if (out_m[li] == ON) {
        out_s = ON;
      } else if (out_m[li] == AUTO) {*/
     if (priorita == 1){
     // if (li < 1){
        if (vState == 0){
          delta = 1;
        } else {
          delta = 0;
        }
        if ((18 + delta) < temperature) {
          digitalWrite(out[0], HIGH);
          //out_s=ON;
          vState = 1;
        } else if (18 > temperature) {
          digitalWrite(out[0], LOW);
          vState = 0;
        } else {
          digitalWrite(out[0], LOW);
          vState = 0;
        }
      //} else if (li < 2){
        if (sState == 0){
          deltaS = 1;
        } else {
          deltaS = 0;
        }
        if (18 - deltaS > temperature) {
          digitalWrite(out[1], HIGH);
          sState = 1; 
        } else if (18 < temperature){
          digitalWrite(out[1], LOW);
          sState = 0;
        } else {
          digitalWrite(out[1], LOW);
          sState = 0;
        }
      //} else if (li < 3){
        if (uState == 0){
          deltaU = 2;
        } else {
          deltaU = 0;
        }
        if ((50 + deltaU) > humidity){
          digitalWrite(out[2], LOW);
          uState = 0;
        } else if (50 < humidity){
          digitalWrite(out[2], HIGH);
          uState = 1;
        } else {
          digitalWrite(out[2], HIGH);
          uState = 1;
        }
       
        digitalWrite(out[3], LOW);
      }
}

void display_day() {
    DateTime now = RTC.now();
    int  dayNow = now.day();
    dayUno = EEPROM.read(60);
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
    lcd.print("    ");
    lcd.setCursor(0, 3);
    lcd.print("Giorno: ");
    lcd.print((dayNow - dayUno) + 1);

   }

  void set_function(byte lnb, byte wpower, byte wpowerN)
  {
    int place, eelocate;
    int pressed_bt = -1;
    int pos = 0;
    int riga = 1;
    int rigaN = 3;
    char val[18];
    byte h1, m1, h2, m2, power, powerN;
    int i;
    int ok = 0;
    place = lnb - 1;
    eelocate = 2+place*6;

    //Serial.print("do set light---------------- Number: ");
    //Serial.print(lnb);
    //Serial.print(" --- with power: ");
    //Serial.print(wpower);
    //Serial.println("---");

    // make sure we are up tu date from EEPROM
    read_eeprom(place);
    h1 = ti[2].h1;
    m1 = ti[2].m1;
    h2 = ti[2].h2;
    m2 = ti[2].m2;
    power = ti[place].power;
    powerN = ti[place].powerN;

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
    val[16] = (wpowerN) ? powerN/10+'0' : ' ';
    val[17] = (wpowerN) ? powerN%10+'0' : ' ';



    lcd.clear();
    //lcd.setCursor(0, 0);
    //lcd.print("Alba  Tramonto");
    if(wpowerN){

      lcd.setCursor(0, 0);
      lcd.print(F("Alba  Tramonto"));
      lcd.setCursor(0, 1);
      for(i = 0; i < 11; i++)
        lcd.print(val[i]);

      // riga di menu per valore giorno notte
      lcd.setCursor(0, 2);
      lcd.print(F("Valore: Giorno Notte"));
      lcd.setCursor(12, 3);
      for(i = 12; i < 18; i++)
        lcd.print(val[i]);
      lcd.setCursor(12, 3);
    }





    else{
      lcd.setCursor(0, 0);
      lcd.print(F("Alba  Tramonto"));
      // solo riga di menu per alba tramonto
      
      lcd.setCursor(0, 1);
      for(i = 0; i < 11; i++)
        lcd.print(val[i]);
    }




    do {
      do {
        // se temperatura umidità
        if (powerN){
          switch(pressed_bt) {
            case BT_LEFT:
              switch(pos) {
                 /* case 1:
                    pos = 0;
                    riga =1;
                    break;
                  case 3:
                    pos = 1;
                    riga =1;
                    break;
                  case 4:
                    pos = 3;
                    riga =1;
                    break;
                  case 6:
                    pos = 4;
                    riga =1;
                    break;
                  case 7:
                    pos = 6;
                    riga =1;
                    break;
                  case 9:
                    pos = 7;
                    riga =1;
                    break;
                  case 10:
                    pos = 9;
                    riga =1;
                    break;
                  case 12:
                    pos = 10;
                    riga =1;
                    break;*/
                  default:
                  //case 1:
                    pos = 12;
                    rigaN =3;
                    break;
                   case 16:
                    pos = 13;
                    rigaN =3;
                    break;
                  case 17:
                    pos = 16;
                    rigaN =3;
                    break;
              }
              break;
            case BT_RIGHT:
              switch(pos) {
                  /*case 0:
                    pos = 1;
                    riga = 1;
                    break;
                  case 1:
                    pos = 3;
                    riga = 1;
                    break;
                  case 3:
                    pos = 4;
                    riga= 1;
                    break;
                  case 4:
                    pos = 6;
                    riga= 1;
                    break;
                  case 6:
                    pos = 7;
                    riga= 1;
                    break;
                  case 7:
                    pos = 9;
                    riga= 1;
                    break;
                  case 9:
                    pos = 10;
                    riga= 1;
                    break;*/
                  default: 
                    // if nothing else matches, do the default
                    // default is optional
                    pos = 12;
                    rigaN= 3;
                    break;
                  case 1:
                    pos = (wpower) ? 12 : 10;
                    rigaN= 3;
                    break;
                  case 12:
                    pos = (wpower) ? 13 : 10;
                    rigaN= 3;
                    break;
                  case 13:
                    pos = (wpowerN) ? 16 : 10;
                    rigaN= 3;
                    break;
                  case 16:
                    pos = (wpowerN) ? 17 : 10;
                    rigaN= 3;
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
        }else{
          switch(pressed_bt) {
          case BT_LEFT:
            switch(pos) {
                case 1:
                  pos = 0;
                  riga =1;
                  break;
                case 3:
                  pos = 1;
                  riga =1;
                  break;
                case 4:
                  pos = 3;
                  riga =1;
                  break;
                case 6:
                  pos = 4;
                  riga =1;
                  break;
                case 7:
                  pos = 6;
                  riga =1;
                  break;
                case 9:
                  pos = 7;
                  riga =1;
                  break;
                case 10:
                  pos = 9;
                  riga =1;
                  break;
                case 12:
                  pos = 10;
                  riga =1;
                  break;
                case 13:
                  pos = 12;
                  riga =3;
                  break;
                 case 16:
                  pos = 13;
                  riga =3;
                  break;
                case 17:
                  pos = 16;
                  riga =3;
                  break;
            }
            break;
          case BT_RIGHT:
            switch(pos) {
                case 0:
                  pos = 1;
                  riga = 1;
                  break;
                case 1:
                  pos = 3;
                  riga = 1;
                  break;
                case 3:
                  pos = 4;
                  riga= 1;
                  break;
                case 4:
                  pos = 6;
                  riga= 1;
                  break;
                case 6:
                  pos = 7;
                  riga= 1;
                  break;
                case 7:
                  pos = 9;
                  riga= 1;
                  break;
                case 9:
                  pos = 10;
                  riga= 1;
                  break;
                case 10:
                  pos = (wpower) ? 12 : 10;
                  riga= 3;
                  break;
                case 12:
                  pos = (wpower) ? 13 : 10;
                  riga= 3;
                  break;
                case 13:
                  pos = (wpowerN) ? 16 : 10;
                  riga= 3;
                  break;
                case 16:
                  pos = (wpowerN) ? 17 : 10;
                  riga= 3;
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
         }
        
        if(val[pos] < '0')
          val[pos] = '9';
        else if (val[pos] > '9')
          val[pos] = '0';
        if(powerN){
          if (pos < 12)
            pos=12;
          lcd.setCursor(pos, rigaN);
          lcd.print(val[pos]);
          lcd.setCursor(pos, rigaN);
        }else{
          lcd.setCursor(pos, riga);
          lcd.print(val[pos]);
          lcd.setCursor(pos, riga);
        }
      } while((pressed_bt = read_button_blocking()) != BT_SET);
      h1 = (val[0]-'0')*10+val[1]-'0';
      m1 = (val[3]-'0')*10+val[4]-'0';
      h2 = (val[6]-'0')*10+val[7]-'0';
      m2 = (val[9]-'0')*10+val[10]-'0';
      power = (wpower) ? (val[12]-'0')*10+val[13]-'0' : 0;
      powerN = (wpowerN) ? (val[16]-'0')*10+val[17]-'0' : 0;

      if(h1 >= 0 && h1 < 24
        && m1 >= 0 && m1 < 60
        && h2 >= 0 && h2 < 24
        && m2 >= 0 && m2 < 60
        && power >= 0 && power <= 99
        && powerN >= 0 && powerN <= 99
        )
                ok = 1;
    } while(!ok);
    ti[place].h1 = h1;
    ti[place].m1 = m1;
    ti[place].h2 = h2;
    ti[place].m2 = m2;
    ti[place].power = power;
    ti[place].powerN = powerN;

    /*EEPROM.write(eelocate++, h1); // H1
    EEPROM.write(eelocate++, m1); // M1
    EEPROM.write(eelocate++, h2); // H2
    EEPROM.write(eelocate++, m2); // M2
    EEPROM.write(eelocate++, power); // P1
    EEPROM.write(eelocate++, powerN);*/
    EEPROM.update(eelocate++, h1); // H1
    EEPROM.update(eelocate++, m1); // M1
    EEPROM.update(eelocate++, h2); // H2
    EEPROM.update(eelocate++, m2); // M2
    EEPROM.update(eelocate++, power); // P1
    EEPROM.update(eelocate++, powerN);
    start_menu();
  }

  /*********************************************************
  **             reads data from EEPROM                   **
  *********************************************************/

  void read_eeprom(byte place)
  {
    int eelocate;
    eelocate = 2+place*6;
    ti[place].h1 = EEPROM.read(eelocate++);
    ti[place].m1 = EEPROM.read(eelocate++);
    ti[place].h2 = EEPROM.read(eelocate++);
    ti[place].m2 = EEPROM.read(eelocate++);
    ti[place].power = EEPROM.read(eelocate++);
    ti[place].powerN = EEPROM.read(eelocate++);
  }

  /************************************************************
  **      this displays the data on the screen               **
  ************************************************************/

  void display_data()
  {

    //float temperature = dht.readTemperature();
    //float humidity = dht.readHumidity();
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
    lcd.print("    ");
    /*lcd.setCursor(0,1);
    lcd.print((char)3);
    lcd.print(F(" "));
    lcd.print(temperature);
    lcd.print((char)223);
    lcd.print(F(" "));
    lcd.print((char)4);
    lcd.print(F(" "));
    lcd.print(humidity);
    lcd.print(F("%"));
    lcd.print(F(" "));
    //lcd.print(':');*/
    //print2dec(now.second());
    // Prints statuses
    for(byte i = 0; i < NBSETS; i++) {
      display_out(i);
    }
  }

  


  /**********************************************************
  **              display modalità dispositivi             **
  **********************************************************/

  void display_out(byte i)
  {
    //byte lampadina[8] = { B01110, B10001, B10101, B10101, B11011, B01110, B01110, B01110 };
    lcd.createChar(5, lampadina);
    //byte fiamma[8] = { B00100, B00110, B01011, B01011, B11110, B11110, B01100, B00100 };
    lcd.createChar(6, fiamma);
    //byte fan[8] = { B00111, B10110, B10100, B01010, B01010, B00101, B01101, B11100 };
    lcd.createChar(7, fan);
    lcd.setCursor(1,2);
    lcd.print((char)7);
    lcd.setCursor(6,2);
    lcd.print((char)6);
    lcd.setCursor(11,2);
    lcd.print((char)4);
    lcd.setCursor(16,2);
    lcd.print((char)5);

    /*char autochar[]= "Auto";
    char offchar[]="Off ";
    char onchar[]="On ";*/
    int x = 0;
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
        lcd.print(F("OFF "));
        //lcd.print(offchar);
        break;
      case AUTO:
        lcd.print(F("AUTO"));
        //lcd.print(autochar);
        break;
      case ON:
        lcd.print(F("ON  "));
        //lcd.print(onchar);
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


void button() {
    int pressed_bt = -1;
    pressed_bt = read_button();

    if (dayUno > 0){
      switch(pressed_bt) {
        case BT_SET:
          chg_status();
          do_menu();
          break;
        }
    } else {
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
     }
 }



 /********************************************************
  **                display temp and humidity            **
  ********************************************************/

  void display_sensor()
  {
        float temperature = dht.readTemperature();
        float humidity = dht.readHumidity();
        //receive();
        lcd.setCursor(0,1);
        lcd.print((char)3);
        lcd.print(F(" "));
        lcd.print(temperature);
        lcd.print((char)223);
        lcd.print(F(" "));
        lcd.print((char)4);
        lcd.print(F(" "));
        lcd.print(humidity);
        lcd.print(F("%"));
        lcd.print(F(" "));
      }