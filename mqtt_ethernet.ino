/***************************************************
  Adafruit MQTT Library Ethernet Example

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Alec Moore
  Derived from the code written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <SPI.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

#include <Ethernet.h>
#include <EthernetClient.h>
#include <Dns.h>
#include <Dhcp.h>
#include <Servo.h>
#include <OneWire.h>
#include <DallasTemperature.h>


//motion sensor
int pirPin = 2;  // PIR Out pin 
uint32_t pirStat = 0; // PIR status
int ledred = 3;
int ledgreen = 4;
uint32_t currenttemp = 0; //current temperature value
uint32_t doorstatus = 0; // status of the door

/************************* Ethernet Client Setup *****************************/
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

//Uncomment the following, and set to a valid ip if you don't have dhcp available.
//IPAddress iotIP (192, 168, 0, 42);
//Uncomment the following, and set to your preference if you don't have automatic dns.
//IPAddress dnsIP (8, 8, 8, 8);
//If you uncommented either of the above lines, make sure to change "Ethernet.begin(mac)" to "Ethernet.begin(mac, iotIP)" or "Ethernet.begin(mac, iotIP, dnsIP)"


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME  "chinchan"
#define AIO_KEY       "aio_Ayat87pPYQJyOZHFcFpuN099turE"

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


/************ Global State (you don't need to change this!) ******************/

//Set up the ethernet client
EthernetClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }


/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish Touch = Adafruit_MQTT_Publish(&mqtt,  "chinchan" "/feeds/Touch");
Adafruit_MQTT_Publish Motion = Adafruit_MQTT_Publish(&mqtt,  "chinchan" "/feeds/Motion");
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt,  "chinchan" "/feeds/Temperature");
Adafruit_MQTT_Publish DoorStatus = Adafruit_MQTT_Publish(&mqtt,  "chinchan" "/feeds/DoorStatus");

// Setup a feed called 'onoff' for subscribing to changes.
Adafruit_MQTT_Subscribe onoffbutton = Adafruit_MQTT_Subscribe(&mqtt, "chinchan" "/feeds/onoff");

/*************************** Sketch Code ************************************/

void setup() {
  Serial.begin(115200);

  Serial.println(F("Adafruit MQTT demo"));

  // Initialise the Client
  Serial.print(F("\nInit the Client..."));
  Ethernet.begin(mac);
  delay(1000); //give the ethernet a second to initialize
  

  mqtt.subscribe(&onoffbutton);


  //motion sensor


  // make the pushbutton's pin an input:
  pinMode(pirPin, INPUT);
  pinMode(ledred, OUTPUT);
  pinMode(ledgreen, OUTPUT);

  // put your setup code here, to run once:
  door.attach(9);

  sensors.begin();
}

uint32_t x=0;

void loop() {

   // read the input pin:
  int pirState = digitalRead(pirPin);
  // print out the state of the button:
  Serial.println(pirState);
  delay(1);        // delay in between reads for stability

  //check internal side
  if(pirState==0){
    digitalWrite(ledred,HIGH);
    digitalWrite(ledgreen,LOW);
    doorstatus = 0;
  }
  if(pirState==1){
    digitalWrite(ledgreen,HIGH);
    digitalWrite(ledred,LOW);
    doorstatus = 1;
    door.write(0);
    delay(1000);
    door.write(100);
    delay(1000);
  }

  //check temperature
  sensors.requestTemperatures(); // Send the command to get temperatures
  //Serial.print(sensors.getTempCByIndex(0));

  // get current temperature
  currenttemp = sensors.getTempCByIndex(0);
  
  //check external side
  if(currenttemp < 37 && digitalRead(6)==HIGH){
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
  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(1000))) {
    if (subscription == &onoffbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *)onoffbutton.lastread);
    }
  }

  // publish motion sensor value
  pirStat = digitalRead(pirPin);
  if (! Motion.publish(pirStat)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // publish temperature value
  if (! Temperature.publish(currenttemp)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // publish door status value
  if (! DoorStatus.publish(doorstatus)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }

  delay(5000);

}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
