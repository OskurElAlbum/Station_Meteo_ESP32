#include <Arduino.h>
//librairie communication
#include <WiFi.h>
#include <MQTT.h>
#include <ArduinoJson.h>
//librairie capteur
#include "Adafruit_BME680.h"
#include "Adafruit_Sensor.h"
#include "lum.h"
//librarie timer
#include "ESP32TimerInterrupt.h"

#define TIMER0_INTERVAL_MS  20000


ESP32Timer ITimer0(0);

const char* ssid ="Maison_Rosnoblet";
const char* password ="ZorroDonDiego";

#define ADRESSE_BROKER "192.168.1.2"


Adafruit_BME680 bme;
WiFiClient client;
MQTTClient MQTTcli;

int GPIOLum = 35;//Pin de la photorésistance

String strLum;  //Valeur envoyer aux broker MQTT
String strTemp;
String strHumidity;
String strPressure;
String strGas;

int iLuminosite;  //Valeur quantifiable
float fTemp;
float fPressure;
float fHumidity;
float fGas;

bool boul;
bool bret;

int i;

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
  MQTTcli.setKeepAlive(50);
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

	// Interval in microsecs
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
  //GPIO Setup
  pinMode(GPIOLum,INPUT);
}

void loop() {
  if (!bme.performReading()) {
    Serial.println("Failed to perform reading :(");
  return;
  }
  fTemp = bme.temperature;//transformation en valeur compréhensible
  fHumidity =bme.humidity;
  fPressure=(bme.pressure / 100.0);
  fGas=(bme.gas_resistance / 1000.0);
  iLuminosite=lux(3.3,12,GPIOLum);

  strTemp= (String) fTemp;
  strHumidity = (String) fHumidity;
  strPressure =(String)fPressure;
  strGas = (String)fGas;
  strLum=(String) iLuminosite;

    if(WiFi.status()==WL_CONNECTED)
    {
        
        DynamicJsonDocument document(1064); //Convertie les valeurs des capteurs en chaine de caractère au format JSON
        i++;
        document["time"]=i;
        document["temperature"]=strLum;
        String charenvoie;
        serializeJson(document,charenvoie);
        
      if(boul==true){                     //Envoie de la data
        bret=MQTTcli.publish("Meteo/Luminosite",charenvoie,1,1);
        Serial.println(bret);
        boul=false;
      }
    }
    else
    {
    Serial.println("WiFi Disconnected");
    }
}