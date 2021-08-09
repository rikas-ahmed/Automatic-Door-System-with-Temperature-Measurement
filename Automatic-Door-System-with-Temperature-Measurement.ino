#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// pir sensor
int pir = 2;
int ledred = 3;
int ledgreen = 4;

// servo 
Servo door;

// temperature
// Data wire is plugged into pin 2 on the Arduino 
#define ONE_WIRE_BUS 5 
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);


// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the pushbutton's pin an input:
  pinMode(pir, INPUT);
  pinMode(ledred, OUTPUT);
  pinMode(ledgreen, OUTPUT);

  // put your setup code here, to run once:
  door.attach(9);

  sensors.begin();
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
  int pirState = digitalRead(pir);
  // print out the state of the button:
  Serial.println(pirState);
  delay(1);        // delay in between reads for stability

  //check internal side
  if(pirState==0){
    digitalWrite(ledred,HIGH);
    digitalWrite(ledgreen,LOW);
  }
  if(pirState==1){
    digitalWrite(ledgreen,HIGH);
    digitalWrite(ledred,LOW);
    door.write(0);
    delay(1000);
    door.write(100);
    delay(1000);
  }

  //check temperature
  sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.print(sensors.getTempCByIndex(0));

  //check external side
  if(sensors.getTempCByIndex(0)<37 && digitalRead(6)==HIGH){
    //Green LED
    digitalWrite(ledgreen, HIGH);
    digitalWrite(ledred, LOW);
    //Rotate servo 90
    //Rotate servo -90
    door.write(0);
    delay(3000);
    door.write(100);
    delay(1000);
  }
  else{
    //Red LED
    digitalWrite(ledgreen, LOW);
    digitalWrite(ledred, HIGH);
  }

}
 
