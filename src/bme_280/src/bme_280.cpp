#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10

#define PRESENCE_LED_DELAY 100
#define SEALEVELPRESSURE_HPA 1013.25


Adafruit_BME280 bme;

unsigned long delayTime = 30000;
unsigned long loop_start = 0;

// UDP Client
WiFiUDP udp;
unsigned int localUdpPort = 4210;

byte mac[6];


// Support functions
void goToSleep();
int setupWifi();
void sendMeasures(JsonObject* root);

void setup() {
  Serial.begin(115200);
  delay(100);

  // BME 280
  bool status = bme.begin();
  if (!status) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1){
      delay(10000);
    }
  }
  
  Serial.println("BME280 Test");
  Serial.println();
  delay(100);



  // fix delay time to avoid calculation on the loop
  if ((delayTime - PRESENCE_LED_DELAY) > 0) {
    delayTime = delayTime - PRESENCE_LED_DELAY;
  } else {
    delayTime = 0;
  }

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);


  // WiFi Connect
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  // Wait a second for wifi connection
  delay(1000);
}

void loop() {

  // JSON Buffer
  StaticJsonBuffer<100> jsonBuffer;

  // Read BME280 values
  float t = bme.readTemperature();
  float h = bme.readHumidity();
  float p = bme.readPressure() / 100.0F;

  Serial.print("Timestamp: ");
  Serial.print(loop_start);
  Serial.print(", Temperature: ");
  Serial.print(t);
  Serial.print("C, Humidity: ");
  Serial.print(h);
  Serial.print("%, Pressure: ");
  Serial.print(p);
  Serial.println("hpa");

  // Publish UDP Packets
  // Temperature
  JsonObject& root = jsonBuffer.createObject();
  root["type"] = "pub";
  root["timestamp"] = loop_start;


  root["topic"] = "temperature";
  root["value"] = t;
  sendMeasures(&root);

  // Humidity
  root["topic"] = "humidity";
  root["value"] = h;
  sendMeasures(&root);

  // Pressure
  root["topic"] = "pressure";
  root["value"] = p;
  sendMeasures(&root);

  // All Done, disable led and to to sleep
  goToSleep();
}


void sendMeasures(JsonObject* root) {

    // Always verify Wifi connection
    if (!setupWifi()) {
      Serial.print("\nNot Connected!");    
      // Sleep until next loop
      delay(delayTime);
      return;
    }


    loop_start = millis();

    // Turn on builtin LED
    digitalWrite(LED_BUILTIN, HIGH);



    udp.beginPacket(GATEWAY_ADDR, GATEWAY_PORT);
    root->printTo(udp);
    udp.println();
    udp.endPacket();

}

int setupWifi() {

  if (WiFi.status() != WL_CONNECTED) {
    // WiFi Connect
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    Serial.print("\n\r \n\rWorking to connect [");
    Serial.print(WiFi.status());
    Serial.print("]");

    // Wait for connection
    int counter = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      counter++;
      if (counter > 30 || WiFi.status() == WL_NO_SSID_AVAIL) {
        return 0;
      }
    }
    Serial.println("");
    Serial.println("NodeMCU(ESP8266)");
    Serial.print("Connected to ");
    Serial.println(WIFI_SSID);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Start UDP Connection
    udp.begin(localUdpPort);

    // Compute the device unique id based on the MAC Address
    WiFi.macAddress(mac);
  }


  return 1;

}


void goToSleep() {
  unsigned long interval = millis() - loop_start;

  if (interval < PRESENCE_LED_DELAY) {
    interval = PRESENCE_LED_DELAY - interval;
    delay(interval);
  }

  digitalWrite(LED_BUILTIN, LOW);

  // Sleep until next loop
  delay(delayTime);
}
