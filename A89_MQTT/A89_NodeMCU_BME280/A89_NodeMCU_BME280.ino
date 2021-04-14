/************************************************************************************************* 
                                      PROGRAMMINFO
************************************************************************************************** 
Funktion: WEMOS D1 R2 mini MQTT Node mit BME280 
**************************************************************************************************
Version: 14.04.2021
**************************************************************************************************
Board: WEMOS D1 R2 mini
**************************************************************************************************
C++ Arduino IDE V1.8.13
**************************************************************************************************
Einstellungen:
https://dl.espressif.com/dl/package_esp32_index.json
http://dan.drown.org/stm32duino/package_STM32duino_index.json
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json
http://arduino.esp8266.com/stable/package_esp8266com_index.json

**************************************************************************************************
Librarys
AsyncMqttClient.h
ESPTCP.h

**************************************************************************************************

**************************************************************************************************/


#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>

#define WIFI_SSID "R2-D2"
#define WIFI_PASSWORD "xxx"

// Raspberri Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(192, 168, 1, 133)
// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
#define MQTT_PORT 1883

// MQTT Topics
#define MQTT_PUB_TEMP "esp/bme280/temperature"
#define MQTT_PUB_HUM "esp/bme280/humidity"
#define MQTT_PUB_PRES "esp/bme280/pressure"

// BME280 I2C
Adafruit_BME280 bme;
// Variables to hold sensor readings
float temp;
float hum;
float pres;

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

unsigned long previousMillis = 0;   
const long interval = 10000;        // Intervall, in dem Sensormesswerte veröffentlicht werden sollen

void connectToWifi() {
  Serial.println("Verbinde mit Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); 
  wifiReconnectTimer.once(2, connectToWifi);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (WiFi.isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void onMqttPublish(uint16_t packetId) {
  Serial.print("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  
  // Initialize BME280 sensor 
  if (!bme.begin(0x76)) {
    Serial.println("Es konnte kein gültiger BME280-Sensor gefunden werden!");
    while (1);
  }
  
  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  //mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
 
  // Broker Authentifizierung
  //mqttClient.setCredentials("USER", "PASSWORD");
  
  connectToWifi();
}

void loop() {
  unsigned long currentMillis = millis();
  // Jede X-Anzahl von Sekunden (Intervall = 10 Sekunden) 
  // wird eine neue MQTT-Nachricht veröffentlicht)
  if (currentMillis - previousMillis >= interval) {
    
    previousMillis = currentMillis;
    // Neue BME280 Sensor Werte
    temp = bme.readTemperature();
    //temp = 1.8*bme.readTemperatur() + 32;
    hum = bme.readHumidity();
    pres = bme.readPressure()/100.0F;
    
    // Veröffentlichen einer MQTT-Meldung topic esp/bme280/temperature
    uint16_t packetIdPub1 = mqttClient.publish(MQTT_PUB_TEMP, 1, true, String(temp).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_TEMP, packetIdPub1);
    Serial.printf("Message: %.2f \n", temp);

    // Veröffentlichen einer MQTT-Nachricht topic esp/bme280/humidity
    uint16_t packetIdPub2 = mqttClient.publish(MQTT_PUB_HUM, 1, true, String(hum).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_HUM, packetIdPub2);
    Serial.printf("Message: %.2f \n", hum);

    // Veröffentlichen einer MQTT-Meldung topic esp/bme280/pressure
    uint16_t packetIdPub3 = mqttClient.publish(MQTT_PUB_PRES, 1, true, String(pres).c_str());                            
    Serial.printf("Publishing on topic %s at QoS 1, packetId: %i ", MQTT_PUB_PRES, packetIdPub3);
    Serial.printf("Message: %.3f \n", pres);
  }
}
