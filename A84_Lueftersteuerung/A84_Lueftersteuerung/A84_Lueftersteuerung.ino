/* 
PROGRAMMINFO

Funktion: NANO Lüftersteuerung

Version: 27.03.2021

() C++ Arduino IDE V1.8.13

Board: NANO V3


Einstellungen:
https://dl.espressif.com/dl/package_esp32_index.json
http://dan.drown.org/stm32duino/package_STM32duino_index.json
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json

 
 */




//Konstante Werte definieren
#define ONE_WIRE_BUS  10                             // D10 pin, verbunden mit DS18B20 DQ-pin 
#define waitTime      500                            // Aktualisierungszeit in ms
#define Tmax          35.0                           // max. Temperatur
#define Tmin          20.0                           // min. Temperatur
#define FAN1          3                              // D3 PWM pin
#define FAN2          5                              // D5 PWM pin
#define FAN3          6                              // D6 PWM pin
#define FAN4          9                              // D9 PWM pin

//************************************************************************************************************

//Bibliotheken einbinden
#include <OneWire.h>  
#include <DallasTemperature.h>

//************************************************************************************************************

//Objekte initialisieren
OneWire oneWire(ONE_WIRE_BUS);                       // OneWire initialisierung
DallasTemperature sensors(&oneWire);                 // DallasTemperature initialisierung

float   Temperature;                                 // Temperatur Variable
int     FanSpeed;                                    // Lüfter geschwindigkeit 
int     FANarray[] = {FAN1, FAN2, FAN3, FAN4};

//************************************************************************************************************

void setup() {
 Serial.begin(57600);                                // Serielle Schnittstelle(USB) initialisieren
 Serial.println("Begin");
 
 sensors.begin();                                    // Tempratur-Biblitothek starten

 //LÃ¼fter EinzelfunktionsprÃ¼fung
 Serial.println("Lueftertest");   
 for (int i=0;i<4;i++)
 {
  pinMode(FANarray[i], OUTPUT);  
  digitalWrite(FANarray[i], HIGH);                    // Volle LÃ¼ftergeschwindigkeit setzen
  delay(2000);
  digitalWrite(FANarray[i], LOW);                     // Lüfter auschalten
 }
 Serial.println("Lueftertest Ende");    
}

//************************************************************************************************************

void loop() {
  sensors.requestTemperatures();                      // Temperatur lesen
  Temperature = sensors.getTempCByIndex(0);           // Temperatur vom Sensor 0 auslesen  
  Serial.print("Temperatur: ");                       // Werte Ã¼ber RS232 Port ausgeben
  Serial.println(Temperature);       
  
  FanSpeed = map(Temperature, Tmin, Tmax, 0, 255);    // Tmin->0% // Tmax->100%
  Serial.print("Lueftergeschwindigkeit: ");           // Werte über RS232 Port ausgeben
  
  if (FanSpeed<25)   FanSpeed = 0;
  if (FanSpeed>255) FanSpeed = 255;
  Serial.println(FanSpeed);        
  
  for (int i=0;i<4;i++)
  {
    analogWrite(FANarray[i], FanSpeed);               // PWM Geschwindigkeit setzen
  }

  delay(waitTime);  
}

//************************************************************************************************************
