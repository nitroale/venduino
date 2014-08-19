//MENWIZ ESAMPLE
#include <Wire.h>
//INSERT ALL THE FOLLOWING 5 INCLUDES AFTER INCLUDING WIRE LIB 
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <buttons.h>
#include <MENWIZ.h>
#include <EEPROM.h>    // to be included only if defined EEPROM_SUPPORT
#include <DHT.h>
DHT dht;

const int sensorPin = A0;
int sensorValue = 0;
int ledVentole = 2;                       // Ventole
//int ledVentoleM = 5;                      // Ventole Manuale
int ledSerp = 3;                          // Serpentina
int tempSoglia = 23;
int addButton = 24;
int addButtonState = 0;         // current state of the button
int lastAddButtonState = 0;     // previous state of the button
int downButton = 28;    //pulsante diminuisce temperatura accensione ventole
int downButtonState = 0;         // current state of the button
int downAddButtonState = 0;     // previous state of the button
int lastDownButtonState = 0;  

int long time=millis();
  int long letturadati=millis();

  int long letturaTemp = millis();
  
  


// DEFINE ARDUINO PINS FOR THE NAVIGATION BUTTONS
#define UP_BUTTON_PIN       22
#define DOWN_BUTTON_PIN     29
#define LEFT_BUTTON_PIN     28 
#define RIGHT_BUTTON_PIN    24
#define CONFIRM_BUTTON_PIN  30
#define ESCAPE_BUTTON_PIN   26 

//Create global object menu and lcd
menwiz menu;
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); // Addr, En, Rw, Rs, d4, d5, d6, d7, backlighpin, polarity

//instantiate global variables to bind to menu
int      tp=0;
float    f=26.0;
boolean  bb=0;
byte     b=50;

void setup(){
  _menu *r,*s1,*s2;
  _var *v; 
  int  mem;

  Serial.begin(9600);  
  
  // have a look on memory before menu creation
  Serial.println(sizeof(menwiz));
  mem=menu.freeRam();
  
  // inizialize the menu object (20 colums x 4 rows)
  menu.begin(&lcd,20,4);

  //create the menu tree
  r=menu.addMenu(MW_ROOT,NULL,F("MENU"));              //create a root menu at first (required)
    s1=menu.addMenu(MW_SUBMENU,r,F("OPZIONI"));     //add a child (submenu) node to the root menu
    
      s2=menu.addMenu(MW_VAR,s1,F("Temperatura"));            //add a terminal node (that is "variable"); 
          s2->addVar(MW_LIST,&tp);                          //create a variable of type "option list".. 
          s2->addItem(MW_LIST,F("option 1"));               //add option to the OPTION LIST
          //s2->addItem(MW_LIST,F("option 2"));               //add option to the OPTION LIST
          //s2->addItem(MW_LIST,F("option 3"));               //add option to the OPTION LIST
          //s2->addItem(MW_LIST,F("option 4"));               //add option to the OPTION LIST
          //s2->addItem(MW_LIST,F("option 5"));               //add option to the OPTION LIST
//s2->setBehaviour(MW_SCROLL_HORIZONTAL,true);    
//          s2->setBehaviour(MW_LIST_2COLUMNS,true);          
//          s2->setBehaviour(MW_LIST_2COLUMNS,true);          

      s2=menu.addMenu(MW_VAR,s1,F("Test float var"));       //add a terminal node (that is "variable"); 
          s2->addVar(MW_AUTO_FLOAT,&f,11.00,100.00,0.5); //create a variable of type "float number"... 
                                                         //...associated to the terminal node and bind it to the app variable "f" of type float
      s2=menu.addMenu(MW_VAR,s1,F("Test byte var"));        //add a terminal node (that is "variable"); 
          s2->addVar(MW_AUTO_BYTE,&b,25,254,10);         //create a variable of type "byte"...
                                                         //...associated to the terminal node and bind it to the app variable "b" of typr byte
      s2=menu.addMenu(MW_VAR,s1,F("Test boolean var"));     //add a terminal node (that is "variable"); 
          s2->addVar(MW_BOOLEAN,&bb);                    //create a variable of type "boolean" 
                                                         //...associated to the terminal node and bind it to the app variable "bb" of type boolean
    s1=menu.addMenu(MW_VAR,r,F("WRITE TO SERIAL"));             //add a terminal node (that is "variable") create an "action" associated to the terminal node... 
      s1->addVar(MW_ACTION,act);                         //the act function as default will be called when enter button is pushed
//      s1->setBehaviour(MW_ACTION_CONFIRM,false);         //...if you don't need action confirmation

    s1=menu.addMenu(MW_VAR,r,F("SAVE TO EPROM"));           //add a terminal node (that is "variable") create an "action" associated to the terminal node... 
      s1->addVar(MW_ACTION,savevar);                     //the act function as default will be called when enter button is pushed

    s1=menu.addMenu(MW_VAR,r,F("LOAD FROM EEPROM"));        //add a terminal node (that is "variable") create an "action" associated to the terminal node... 
      s1->addVar(MW_ACTION,loadvar);                     //the act function as default will be called when enter button is pushed

  //declare navigation buttons (required)
  menu.navButtons(UP_BUTTON_PIN,DOWN_BUTTON_PIN,LEFT_BUTTON_PIN,RIGHT_BUTTON_PIN,ESCAPE_BUTTON_PIN,CONFIRM_BUTTON_PIN);

  //(optional)create a user define screen callback to activate after 10 secs (10.000 millis) since last button push 
  menu.addUsrScreen(msc,10000);

  //(optional) create a splash screen (duration 5.000 millis)with some usefull infos the character \n marks end of LCD line 
  //(tip): use preallocated internal menu.sbuf buffer to save memory space!
  sprintf(menu.sbuf,"MENWIZ TEST V %s\n.Free mem. :%d\n.Used mem  :%d\n.Lap secs  :%d",menu.getVer(),menu.freeRam(),mem-menu.freeRam(),5);
  menu.addSplash((char *) menu.sbuf, 5000);
  
  
  dht.setup(A0);
 delay(dht.getMinimumSamplingPeriod());
 
  for(int i = 0; i< 3; i++){
   pinMode(ledVentole, OUTPUT);          // Ventole Uscita Automatica
 // pinMode(ledVentoleM, OUTPUT);         // Ventole Manuale Uscita
  pinMode(ledSerp, OUTPUT);             // Serpentina Uscita Automatica
 // pinMode(ledSerpM, OUTPUT);            // Serpentina Manuale Uscita
 pinMode(sensorPin, INPUT);
 pinMode(addButton, INPUT);
 pinMode(downButton, INPUT);
  }

 
  }
  

void loop(){
  
  
 
  
  
  
  menu.draw(); 
  //PUT APPLICATION CODE HERE (if any)
  int temperature = dht.getTemperature();
 int humidity = dht.getHumidity();
 int delta = temperature - tempSoglia;
  if (delta >= 1) {  // se temperatura maggiore di 23
    
    digitalWrite(2, HIGH);  // accende la ventola
    digitalWrite(3, LOW);  //resistenza Spenta
    
    
    } else if(delta <= -1) { // se la temperatura è minore di 18
   digitalWrite(2, LOW); // Ventola spenta
    digitalWrite(3, HIGH); // Serpentina Accesa
   
    
    } 
    else {                // se temperatura compresa tra 25 e 18 tutto spento
    digitalWrite(2, LOW);  // Ventole Spente
    digitalWrite(3, LOW);  // Serpentine Spente
   
    }
   // aumenta soglia ventole
  // read the pushbutton input pin:
  addButtonState = digitalRead(addButton);

  // compare the buttonState to its previous state
  if (addButtonState != lastAddButtonState) {
    // if the state has changed, increment the counter
    if (addButtonState == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      tempSoglia++;
      Serial.print("number of button pushes:  ");
      Serial.println(tempSoglia);
    }
  }
  // save the current state as the last state, 
  //for next time through the loop
  lastAddButtonState = addButtonState;
  
  // diminuisce soglia ventole
  // read the pushbutton input pin:
  downButtonState = digitalRead(downButton);

  // compare the buttonState to its previous state
  if (downButtonState != lastDownButtonState) {
    // if the state has changed, increment the counter
    if (downButtonState == HIGH) {
      // if the current state is HIGH then the button
      // wend from off to on:
      tempSoglia--;
      Serial.print("number of button pushes:  ");
      Serial.println(tempSoglia--);
    }
  }
  // save the current state as the last state, 
  //for next time through the loop
  lastDownButtonState = downButtonState;

// user defined callbacks
}

void msc(){
 int temperature = dht.getTemperature();
 int humidity = dht.getHumidity();
  //char buffer[100];
  sprintf(menu.sbuf,"Monitoraggio\nTemperatura: %2d C\nUmidita'   : %d \n\n",temperature,humidity);
  menu.drawUsrScreen(menu.sbuf);
  //lcd.clear();
  //lcd.setCursor(0,1);
  //lcd.print("humidity");
  
  }
  
void act(){
  Serial.println("FIRED ACTION!");
 }
 
void savevar(){
  menu.writeEeprom();
  }
  
void loadvar(){
  menu.readEeprom();
  }

