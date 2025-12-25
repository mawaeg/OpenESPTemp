#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Melopero_RV3028.h>

#include "CredentialManager.h"
#include "ConfigurationWebServer.h"

// The sleeptime in minutes
#define SLEEPTIME 15

#define SDA_PIN 21
#define SCL_PIN 22
#define LATCH_PIN 2
#define VOLTAGE_PIN 0

Adafruit_BME280 bme;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);
Melopero_RV3028 rtc;

CredentialManager manager;
ConfigurationWebServer configServer(manager);

float temperature = -1;
float humidity = -1;
float pressure = -1;
float voltage = -1;

void initialize_wifi();
void initialize_timer();
bool initialize_bme();
void read_data();
void post_data();
void stop_wifi();

void setup() {
  // Hold up the Power Latch
  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, HIGH);

  delay(1000);

  // Bat LVL
  pinMode(0, INPUT);
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Hello world!");
  #endif

  if (!manager.is_valid()){
    configServer.run();
  }

  // In case no connection is established esp sleeps 15 minutes.
  initialize_wifi();

  Wire.begin(SDA_PIN, SCL_PIN);
  initialize_timer();
  bool init_successful = initialize_bme();
  #ifdef DEBUG
    Serial.println("----");
  #endif

  // If sensor was initialized successful we want to read an post data.
  if (init_successful){
    read_data();
    post_data();
  }

  stop_wifi();

  digitalWrite(LATCH_PIN, LOW);
}

void loop() {
  // This will never be called and therefore stay empty as the esp goes back to sleep after sending data in setup.
  delay(1000);
}

void initialize_wifi(){
  #ifdef DEBUG
    Serial.println("Connecting to network.");
  #endif
  WiFi.setHostname("ESP32-C6-Temp");
  WiFi.begin(manager.get(SSID), manager.get(PASSWORD));

  uint8_t connect_counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    connect_counter++;
    if(connect_counter >= 10){
      // Sleep after 10 seconds
      #ifdef DEBUG
        Serial.println("Unable to connect to Wifi. Sleeping.");
      #endif
      digitalWrite(LATCH_PIN, LOW);
    }
    delay(1000);
    #ifdef DEBUG
      Serial.println("Connecting...");
    #endif
  }
  #ifdef DEBUG
    Serial.print("Connected with ip: ");
    Serial.println(WiFi.localIP());
  #endif
}

void initialize_timer(){
  rtc.initI2C();
  rtc.set24HourMode();
  rtc.setTime(2025, 12, 3, 5, 0, 0, 0);
  rtc.setDateModeForAlarm(false);
  rtc.enableAlarm(0, rtc.getHour(), rtc.getMinute() + SLEEPTIME, false, true, true, true);
  rtc.clearInterruptFlags();

  #ifdef DEBUG
    Serial.println("Clock and Alarm successfully set.");
  #endif
}

bool initialize_bme(){
  #ifdef DEBUG
    Serial.println("Initializing BME280");
  #endif
  int status = bme.begin(0x76, &Wire);
  bme.setSampling(Adafruit_BME280::MODE_FORCED,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::SAMPLING_X1,
                  Adafruit_BME280::FILTER_OFF);

  if (!status){
    #ifdef DEBUG
      Serial.println("Could not find BME280.");
    #endif
    return false;
  }
  #ifdef DEBUG
    Serial.println("BME280 was successfully initialized!");
  #endif
  return true;
}

void read_data(){
  // Saving data
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  voltage = (analogReadMilliVolts(VOLTAGE_PIN) / 1000.0F)*2.0F;
  #ifdef DEBUG
    Serial.print("Temperature: ");
    Serial.println(temperature);
    Serial.print("Humidity: ");
    Serial.println(humidity);
    Serial.print("Pressure: ");
    Serial.println(pressure);
  #endif

  Wire.end();
}

void post_data(){
  #ifdef DEBUG
    Serial.println("Sending request");
  #endif
  HTTPClient httpClient;

  httpClient.setConnectTimeout(5000);
  //We do not need to set timeout as it is set to 5000 by default.

  #ifdef DEBUG
    Serial.println("Preparing data");
  #endif

  httpClient.begin(manager.get(POST_URL));
  httpClient.addHeader("Authorization", "Bearer " + manager.get(AUTHORIZATION));
  httpClient.addHeader("Content-Type", "application/json");

  String json_data = "{\"temperature\": \"" + String(temperature) + "\", \"humidity\": \"" + String(humidity) + "\", \"pressure\": \"" + String(pressure) + "\", \"voltage\": \"" + String(voltage) + "\"}";


  #ifdef DEBUG
    Serial.print("Sending request: ");
  #endif
  int request = httpClient.POST(json_data);
  #ifdef DEBUG
    Serial.println(request);
  #endif
  httpClient.end();

  #ifdef DEBUG
    Serial.println("Request was sent");
  #endif
}

void stop_wifi(){
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}