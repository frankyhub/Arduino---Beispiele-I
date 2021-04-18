/************************************************************************************************* 
                                      PROGRAMMINFO
************************************************************************************************** 
Funktion: WEMOS D1 R2 mini MQTT Node A89_NodeMCU_Multisensor
**************************************************************************************************
Version: 14.40.2021
**************************************************************************************************
Board: WEMOS D1 R2 mini
**************************************************************************************************
C++ Arduino IDE V1.8.13
**************************************************************************************************
Einstellungen:
https://dl.espressif.com/dl/package_esp32_index.json
http://dan.drown.org/stm32duino/package_STM32duino_index.json
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
**************************************************************************************************
Librarys
- PubSubClient.h

LDR ->    A0
Relais -> D0
PIR->     D1
DS18B20-> D2 
LED ->    D6

**************************************************************************************************/



#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>


const char* ssid     = "R2-D2";
const char* password = "xxx";


const char* mqtt_server = "192.168.1.110";



WiFiClient espClient;
PubSubClient client(espClient);

String temperatureString = "";

const int output = 15;
const int statusLed = 12;
const int motionSensor = 5;
const int ldr = A0;

String outputState = "off";


const int oneWireBus = 4;          

OneWire oneWire(oneWireBus);
 
DallasTemperature sensors(&oneWire);

unsigned long now = millis();
unsigned long lastMeasure = 0;
boolean startTimer = false;
unsigned long currentTime = millis();
unsigned long previousTime = 0; 

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(topic=="esp8266/output"){
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      digitalWrite(output, LOW);
      Serial.print("on");
    }
    else if(messageTemp == "off"){
      digitalWrite(output, HIGH);
      Serial.print("off");
    }
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())) {
      Serial.println("connected");  

      client.subscribe("esp8266/output");  
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


ICACHE_RAM_ATTR void detectsMovement() {
  Serial.println("MOTION DETECTED!");
  client.publish("esp8266/motion", "MOTION DETECTED!");
  previousTime = millis();
  startTimer = true;
}

void setup() {

  sensors.begin();

  Serial.begin(115200);

  pinMode(motionSensor, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  
  pinMode(output, OUTPUT);
  pinMode(statusLed, OUTPUT);
  digitalWrite(output, HIGH);
  digitalWrite(statusLed, LOW);
  
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  now = millis();

  if (now - lastMeasure > 30000) {
    lastMeasure = now;
    sensors.requestTemperatures(); 

    temperatureString = String(sensors.getTempCByIndex(0));
    client.publish("esp8266/temperature", temperatureString.c_str());
    Serial.println("Temperature published");
    
    client.publish("esp8266/ldr", String(analogRead(ldr)).c_str());
    Serial.println("LDR values published");    
  }

  if ((now - previousTime > 10000) && startTimer) { 
    client.publish("esp8266/motion", "No motion");
    Serial.println("Motion stopped");
    startTimer = false;
  }
}
