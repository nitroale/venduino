#include <VirtualWire.h>


 #include <DHT.h>
  //DHT dht;
  #define DHTPIN A0     // what pin we're connected to
  #define DHTTYPE DHT22   // DHT 11
  DHT dht(DHTPIN, DHTTYPE);
  void setup(){
   Serial.begin(9600);
   dht.begin();
  }
void loop()
{
    float tempC  = dht.readTemperature();
    int tempC1 = (int)tempC;
    int tempC2 = (int)(tempC - tempC1) * 100; // For two decimal points
    char msg[24];
    sprintf(msg, "%i.%i", tempC1,tempC2);
    vw_send((uint8_t *)msg, strlen(msg));
    vw_wait_tx();                            // Wait for message to finish
    delay(200);
}
