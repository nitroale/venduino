// Project 6 - Creating a Single-Cell Battery Tester
#define newLED 2 // green LED 'new'
#define okLED 4 // yellow LED 'ok'
#define oldLED 6 // red LED 'old'
int analogValue = 0;

float voltage = 0;
int ledDelay = 2000;
void setup()
{
pinMode(newLED, OUTPUT);
pinMode(okLED, OUTPUT);
pinMode(oldLED, OUTPUT);
Serial.begin(9600);
}
void loop()
{

analogValue = analogRead(0);

voltage = 0.0048*analogValue;
Serial.println(voltage);
if ( voltage >= 1.3 )
{
digitalWrite(newLED, HIGH);
delay(ledDelay);
digitalWrite(newLED, LOW);
}

else if ( voltage < 1.6 && voltage > 1.15 )
{
digitalWrite(okLED, HIGH);
delay(ledDelay);
digitalWrite(okLED, LOW);
}

else if ( voltage <= 1.2 )
{
digitalWrite(oldLED, HIGH);
delay(ledDelay);
digitalWrite(oldLED, LOW);
}
}
