#include <Arduino.h>
//librairie communication
#include <WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>
//librairie capteur
#include "Adafruit_BME680.h"
#include "Adafruit_Sensor.h"
#include "lum.h"
//librairie timer
#include "ESP32TimerInterrupt.h"
//librarie temps
#include <WiFiUdp.h>
#include <NTPClient.h>

//IP Broker
#define ADRESSE_BROKER "192.168.1.2"

//Identifiant WiFi
const char* ssid ="Maison_Rosnoblet";
const char* password ="ZorroDonDiego";

ESP32Timer ITimer0(0);
Adafruit_BME680 bme;
WiFiClient client;
MQTTClient MQTTcli;
WiFiUDP ntpUDP;
NTPClient TimeCli(ntpUDP,"fr.pool.ntp.org", 3600, 60000);

//Interval entre 2 mesure en ms
#define TIMER0_INTERVAL_MS  120000 

//Pin de la photorésistance
int GPIOLum = 35;

//Valeur envoyer aux broker MQTT
String strLum;  
String strTemp;
String strHumidity;
String strPressure;
String strGas;

//mesure numérique
int iLuminosite;  
float fTemp;
float fPressure;
float fHumidity;
float fGas;

bool boul;
bool bret;

//Horaire mesure
int iHours;
int iMinute;
String strDateMesure;

//Intteruption timer
bool IRAM_ATTR TimerHandler0(void * timerNo)
{
  boul = true;
	return true;
}

void WiFi_Setup(){
    WiFi.begin(ssid, password);
  Serial.print("Connexion au Wifi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("connecté au réseau, adresse ip: ");
  Serial.println(WiFi.localIP());
}

void MQTT_Setup(){
  MQTTcli.begin(ADRESSE_BROKER,1883,client);
  MQTTcli.setKeepAlive(400);
  Serial.print("\nconnecting au broker\n");
  while (!MQTTcli.connect(ADRESSE_BROKER,"Maison","BananaChocolat",false)) {
    Serial.print(".");
    delay(1000);
  }
    Serial.println("connecter");
}

void BME_Setup(){
  while (!Serial); //setup capteur BME680
  Serial.println(F("BME680 test"));
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
   // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void Timer0_Setup(){
  Serial.print(F("\nStarting TimerInterruptTest on "));
	Serial.println(ARDUINO_BOARD);
	Serial.println(ESP32_TIMER_INTERRUPT_VERSION);
	Serial.print(F("CPU Frequency = "));
	Serial.print(F_CPU / 1000000);
	Serial.println(F(" MHz"));

	// Using ESP32  => 80 / 160 / 240MHz CPU clock ,
	// For 64-bit timer counter
	// For 16-bit timer prescaler up to 1024

	// Interruption timer
	if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0))
	{
		Serial.print(F("Starting  ITimer0 OK, millis() = "));
		Serial.println(millis());
	}
	else
		Serial.println(F("Can't set ITimer0. Select another freq. or timer"));
}

void setup() {
  Serial.begin(9600);
  //WiFi Setup
  WiFi_Setup();
  //MQTT Setup
  MQTT_Setup();
  //BMESetup;
  BME_Setup();
  //timer n°0 Setup
  Timer0_Setup();

  TimeCli.begin();
  TimeCli.setTimeOffset(1);//UTC+1 France
  //GPIO Setup
  pinMode(GPIOLum,INPUT);
}

void loop() {
  if (!bme.performReading()) 
  {
    Serial.println("Failed to perform reading :(");
    return;
  }
  if(WiFi.status()==WL_CONNECTED)
  {
    //Envoie de la data
    if(boul==true)
    { 
      //transformation des mesures en valeur numérique
      fTemp = bme.temperature;
      fHumidity =bme.humidity;
      fPressure=(bme.pressure / 100.0);
      fGas=(bme.gas_resistance / 1000.0);
      iLuminosite=lux(3.3,12,GPIOLum);

      //Conversion en chaine de caractère
      strTemp= (String) fTemp;
      strHumidity = (String) fHumidity;
      strPressure =(String)fPressure;
      strGas = (String)fGas;
      strLum=(String) iLuminosite;

      //Met à jour et met au format hh:mm l'heure de la mesure  
      TimeCli.update(); 
      iHours=TimeCli.getHours();
      iMinute=TimeCli.getMinutes();
      strDateMesure=(String)iHours+':'+(String)iMinute;

      //Met les données au format Json
      DynamicJsonDocument document(1064); //Convertie les valeurs des capteurs en chaine de caractère au format JSON
      document["time"]=strDateMesure;
      document["temperature"]=strLum;
      document["Luminosite"]=strTemp;
      document["Humidite"]=strHumidity;
      document["Pression"]=strPressure;
      String charenvoie;
      serializeJson(document,charenvoie);

      //Envoie les données via MQTT
      bret=MQTTcli.publish("Meteo",charenvoie,0,1);
      Serial.println(bret);
      //Permet de faire une mesure à la prochain interruption du timer0
      boul=false;
    }
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }
}