// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
 
#include <Wire.h>
#include "RTClib.h"
 
RTC_DS1307 RTC;
 int led = 9;
 long oraInizio = 22;
 long minutiInizio = 59;
 long oraFine = 3;
 long minutiFine = 42;  

 unsigned long secondiInizio = 0;
 unsigned long secondiFine = 0;
 unsigned long secondiAttuali = 0;


void setup () {
    Serial.begin(9600);
    Wire.begin();
    RTC.begin();
 
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
 
}
 
void loop () {
    DateTime now = RTC.now();
    
    
    
    Serial.print(now.day(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.year(), DEC);
    Serial.print(' ');
    
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
 
    timer();
    
    delay(3000);
}


/******************************************************
    determino accensione in base all'ora
******************************************************/

void timer() {
    // determino che ore sono
     DateTime now = RTC.now();
    // converto orario in secondi
     unsigned long oraInSecondi = now.hour() * 3600L;
     unsigned long minutiInSecondi = now.minute() * 60L;

     secondiAttuali = oraInSecondi + minutiInSecondi;
     secondiInizio = (oraInizio * 3600 + minutiInizio * 60);
     secondiFine = (oraFine * 3600 + minutiFine * 60);
    
    Serial.println(secondiAttuali);
    Serial.print("INIZIO: ");
    Serial.print(oraInizio);
    Serial.print(":");
    Serial.println(minutiInizio);
    Serial.print("FINE: ");
    Serial.print(oraFine);
    Serial.print(":");
    Serial.println(minutiFine);
    
    

   
    if (secondiInizio < secondiFine) { // caso normale 8:00 18:00
      if (secondiInizio < secondiAttuali && secondiAttuali < secondiFine ) {
        digitalWrite(led,HIGH);
      } else {
        digitalWrite(led,LOW);
      }
    } else {                  // caso 18:00 8:00
      if (secondiInizio < secondiAttuali || secondiAttuali < secondiFine) {
        digitalWrite(led, HIGH);
      } else {
        digitalWrite(led, LOW);
      }
    }

    if (led = HIGH){
        Serial.println("ACCESO");
    } else {
        Serial.println("SPENTO");
    }
}