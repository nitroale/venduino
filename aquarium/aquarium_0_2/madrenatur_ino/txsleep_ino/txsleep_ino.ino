/**
Questo programma permette di inviare i valori della temperatura e dell'umidità fuori casa, ad un Arduino, che avrà il compito di caricare
i dati su Xively
Versione 1.0
Autore Giacomo Bellazzi
*/
#include "DataCoder.h"
#include "VirtualWire.h"
#include "DHT.h"
#include "JeeLib.h"
#define DHTPIN A0
#define DHTTYPE DHT22

 
int transmit_pin = 4;
int baudRate = 800;

 
DHT dht(DHTPIN, DHTTYPE);

ISR(WDT_vect) { Sleepy::watchdogEvent(); }
void setup()
{
  Serial.begin(9600);
  SetupRFDataTxnLink(transmit_pin, baudRate);
  dht.begin();
}
 
void loop()
{
  
  float outArray[2];
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  outArray[0] = t;
  outArray[1] = h;
  union RFData outDataSeq;
  EncodeRFData(outArray, outDataSeq);
  TransmitRFData(outDataSeq); 
  Sleepy::loseSomeTime(5000);
  
}
