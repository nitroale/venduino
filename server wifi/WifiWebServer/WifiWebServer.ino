#include <DHT.h>

#include <SPI.h>
#include <WiFi.h>

// EDIT: Change the 'ssid' and 'password' to match your network
char ssid[] = "Telecom-71485413";  // wireless network name
char password[] = "Wb1KU8Rb2hmcDPjrU6zPi9jq"; // wireless password
int status = WL_IDLE_STATUS;
WiFiClient client;

// EDIT: 'Server' address to match your domain
char server[] = "http://alegiuliano.16mb.com"; // This could also be 192.168.1.18/~me if you are running a server on your computer on a local network.
#define DHTPIN A0 // SENSOR PIN
#define DHTTYPE DHT22 // SENSOR TYPE - THE ADAFRUIT LIBRARY OFFERS SUPPORT FOR MORE MODELS
DHT dht(DHTPIN, DHTTYPE);

long previousMillis = 0;
unsigned long currentMillis = 0;
long interval = 250000; // READING INTERVAL

int t = 0;	// TEMPERATURE VAR
int h = 0;	// HUMIDITY VAR
String data;


void setup() {
  Serial.begin(9600);
  
  connectWifi();

  // You're connected now, so print out the status
  printWifiStatus();
  dht.begin(); 
	delay(10000); // GIVE THE SENSOR SOME TIME TO START

	h = (int) dht.readHumidity(); 
	t = (int) dht.readTemperature(); 

	data = "";
  postData();
}

void loop() {
  currentMillis = millis();
	if(currentMillis - previousMillis > interval) { // READ ONLY ONCE PER INTERVAL
		previousMillis = currentMillis;
		h = (int) dht.readHumidity();
		t = (int) dht.readTemperature();
}

void connectWifi() {
  // Attempt to connect to wifi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, password);
    // Wait 10 seconds for connection
    delay(10000);
  }
}

void printWifiStatus() {
  // Print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // Print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // Print the received signal strength
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

data = "temp1=" + t + "&hum1=" + h;

	if (client.connect("www.*****.*************.com",80)) { // REPLACE WITH YOUR SERVER ADDRESS
		client.println("POST /add.php HTTP/1.1"); 
		client.println("Host: *****.*************.com"); // SERVER ADDRESS HERE TOO
		client.println("Content-Type: application/x-www-form-urlencoded"); 
		client.print("Content-Length: "); 
		client.println(data.length()); 
		client.println(); 
		client.print(data); 
	} 

	if (client.connected()) { 
		client.stop();	// DISCONNECT FROM THE SERVER
	}

	delay(300000); // WAIT FIVE MINUTES BEFORE SENDING AGAIN
}
