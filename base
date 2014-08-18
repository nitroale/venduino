#include <DHT.h>                             // Includi Libreria Sensore Temperatura e Umidità

#include <Wire.h> 

#include <LiquidCrystal_I2C.h>                // Includi Libreria Scheda I2C

#include <SPI.h>

#include <WiFi.h>                            // Includi Libreria WiFi

char ssid[] = "Alice-31710539";              //  Nome della Mia Rete

char pass[] = "43t1c970fgiqg7pt3bfj58g5";    // Password della Mia Rete

int keyIndex = 0;                           // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(5500);

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

DHT dht;

const int sensorPin = A0;                 // Sensore Temperatura
int sensorPin1 = A1;                      // Sensore Terreno
int sensorValue=0;                        // Crea una variabile in cui salvarmi i Valori Letti

int ledVentole = 2;                       // Ventole
int ledVentoleM = 5;                      // Ventole Manuale
int ledSerp = 3;                          // Serpentina
int ledSerpM = 22;                        // Serpentina Manuale
int ledTerr = 8;                          // Terreno
int ledTerrM = 23;                        // Terreno Manuale
int ledProva = 9;                         // Prova

void setup() 
{
  Serial.begin(9600);
  
  lcd.begin(20,4);
  {
  for(int i = 0; i< 3; i++)
  pinMode(ledVentole, OUTPUT);          // Ventole Uscita Automatica
  pinMode(ledVentoleM, OUTPUT);         // Ventole Manuale Uscita
  pinMode(ledSerp, OUTPUT);             // Serpentina Uscita Automatica
  pinMode(ledSerpM, OUTPUT);            // Serpentina Manuale Uscita
  pinMode(ledTerr, OUTPUT);             // Terreno Uscita Automatica
  pinMode(ledTerrM, OUTPUT);            // Terreno Uscita Manuale
  pinMode(ledProva, OUTPUT);            // Led di Prova
  pinMode(sensorPin, INPUT);            // Sensore Temperatura e Umidità in entrata
  pinMode(sensorPin1, INPUT);           // Sensore Terreno in entrata
   
  lcd.setCursor(0,0);                   //Inizia da carattere 0 linea 0
  lcd.print("SERRA ALE & MARTA");       //Scrivimi SERRA ALE & MARTA
  lcd.setCursor(0,1);                   //Inizia da carattere 0 linea 1
  lcd.print("Temp. Aria:");             //Scrivimi Temp. Aria
  lcd.setCursor(0,2);                   //Inizia da carattere 0 linea 2
  lcd.print("Umid. Aria:");             //Scrivimi Umid. Aria
  delay(1000);                          //Attendi 1000 Millisecondi

  if (WiFi.status() == WL_NO_SHIELD) {          // controlla la presenza di shield
    Serial.println("WiFi shield not present");  // se non c'è la shield scivi
    while(true);                                // Se no continua
  } 

  while ( status != WL_CONNECTED) {      // attempt to connect to Wifi network:
    Serial.print("Attempting to connect to Network named: ");
    Serial.println(ssid);                   // Stampami Il nome della Rete (SSID);

    status = WiFi.begin(ssid, pass);    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    
    delay(10000);    // Aspetta 10 seconds Per la connessione
  } 
  server.begin();                           // start the web server on port 80
  printWifiStatus();                        // you're connected now, so prin 


  dht.setup(A0);                            // Dati pin A0
}
}



void loop() 
{
  sensorValue = analogRead(sensorPin1);
  Serial.print("Valore: ");
  Serial.println( sensorValue );
  
  if(sensorValue > 700){
    digitalWrite(8, HIGH);
}
  else if(sensorValue < 600){
    digitalWrite(8, LOW);
}
 delay(dht.getMinimumSamplingPeriod());

  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
 lcd.setCursor(0,2);

 lcd.setCursor(12,2);
  lcd.print("% ");
  delay(1000);
  
  lcd.print(humidity, 1);
  lcd.setCursor(12,1);
  lcd.print("C ");
  lcd.print(temperature, 1);
  lcd.setCursor(12,2);
  
  if (temperature > 29) {  // se temperatura maggiore di 29
    
    digitalWrite(2, HIGH);  // accende la ventola
    digitalWrite(3, LOW);  //resistenza Spenta
    lcd.setCursor(0,3);  
    lcd.print("Ventole Accese");
    
    } else if(temperature < 18) { // se la temperatura è minore di 18
    digitalWrite(2, LOW); // Ventola spenta
    digitalWrite(3, HIGH); // Serpentina Accesa
    lcd.setCursor(0,3); // Scrivimi su LCD Al Carattere 0 e Linea 
    lcd.print("Serp. Accese"); // Serpentine Accese
    
    } else {                // se temperatura compresa tra 25 e 18 tutto spento
    digitalWrite(2, LOW);  // Ventole Spente
    digitalWrite(3, LOW);  // Serpentine Spente
    lcd.setCursor(0,3);   // e quindi Scrivimi al carattere 0 e Linea 3
    lcd.print("Tutto Spento  "); // Tutto Spento
    }
    
  delay(1000);  // intervallo tra le letture
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("new client");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {  
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:    
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Marta e Ale Premete <a href=\"/H\">QUI</a> per Accendere Led 9 di Prova<br>");
            client.print("Marta e Ale Premete <a href=\"/L\">QUI</a> per Spegnere Led 9 di Prova<br>");
            client.print("Marta e Ale Premete <a href=\"/M\">QUI</a> per Accendere Led 5 Ventole<br>");
            client.print("Marta e Ale Premete <a href=\"/N\">QUI</a> per Spegnere Led 5 Ventole<br>");
            client.print("Marta e Ale Premete <a href=\"/O\">QUI</a> per Accendere Led 22 Serpentine<br>");
            client.print("Marta e Ale Premete <a href=\"/P\">QUI</a> per Spegnere Led 22 Serpentine<br>");
            client.print("Marta e Ale Premete <a href=\"/Q\">QUI</a> per Accendere Led 23 Pompa Acqua<br>");
            client.print("Marta e Ale Premete <a href=\"/R\">QUI</a> per Spegnere Led 23 Pompa Acqua<br>");
            
            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;         
          } 
          else {      // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        }     
        else if (c != '\r') {    // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(9, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(9, LOW);                // GET /L turns the LED off
        }  
        if (currentLine.endsWith("GET /M")) {
          digitalWrite(5, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /N")) {
          digitalWrite(5, LOW);                // GET /L turns the LED off
        } 
          
        if (currentLine.endsWith("GET /O")) {
          digitalWrite(22, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /P")) {
          digitalWrite(22, LOW);                // GET /L turns the LED off
        }
        if (currentLine.endsWith("GET /Q")) {
          digitalWrite(23, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /R")) {
          digitalWrite(23, LOW);                // GET /L turns the LED off
        }      
      }
    }
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
}
 
