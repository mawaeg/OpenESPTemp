#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#include "CredentialManager.h"
#include "ConfigurationWebServer.h"

uint64_t deepsleep_time = 900000000;

Adafruit_BME280 bme;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

CredentialManager manager;
ConfigurationWebServer configServer(manager);

float temperature = -1;
float humidity = -1;
float pressure = -1;
float voltage = -1;

void initialize_wifi();
bool initialize_bme();
void read_data();
void post_data();
void set_sleep_time();
void stop_wifi();

void setup() {
  pinMode(4, INPUT);
  #ifdef DEBUG
    Serial.begin(115200);
    Serial.println("Hello world!");
  #endif

  if (!manager.is_valid()){
    configServer.run();
  }

  // In case no connection is established esp sleeps 15 minutes.
  initialize_wifi();
  // In case sensor couldn't be initialized we can still calculate correct sleep time and sleep then.
  bool init_successful = initialize_bme();
  #ifdef DEBUG
    Serial.println("----");
  #endif

  // If sensor was initialized successful we want to read an post data.
  if (init_successful){
    read_data();
    post_data();
  }
  // ToDo Send Error message in case bme init failed.

  
  // Calculate correct sleep time with ntp
  set_sleep_time();

  stop_wifi();

  esp_sleep_enable_timer_wakeup(deepsleep_time);

  // Disable unused peripherals
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_OFF);    // RTC peripherals
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_OFF);  // RTC slow memory
  esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_OFF);  // RTC fast memory
  esp_sleep_pd_config(ESP_PD_DOMAIN_XTAL, ESP_PD_OPTION_OFF);          // Crystal oscillator
  esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);           // Ensure that no more power domains are turned on

  esp_deep_sleep_start();
}

void loop() {
  // This will stay never get called and therefore stay empty as the esp goes back to sleep after sending data in setup.
}

void initialize_wifi(){
  #ifdef DEBUG
    Serial.println("Connecting to network.");
  #endif
  WiFi.begin(manager.get(SSID), manager.get(PASSWORD));

  uint8_t connect_counter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    connect_counter++;
    if(connect_counter >= 10){
      //Deepsleep after 10 seconds
      #ifdef DEBUG
        Serial.println("Unable to connect to Wifi. Sleeping.");
      #endif
      ESP.deepSleep(deepsleep_time);
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

bool initialize_bme(){
  #ifdef DEBUG
    Serial.println("Initializing BME280");
  #endif
  Wire.setPins(5, 6);
  int status = bme.begin(0x76);
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
  voltage = (analogReadMilliVolts(4) / 1000.0F)*2.0F;
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

void set_sleep_time(){
  timeClient.begin();
  timeClient.update();

  //Calculate time till next quarter hour.
  uint8_t minutes = 15 - (timeClient.getMinutes() % 15);
  deepsleep_time = (minutes * 60 - timeClient.getSeconds() + 2) * 1000000; //Adding 2 seconds to ensure wakeup ist after %15

  timeClient.end();
  ntpUDP.stop();
  #ifdef DEBUG
    Serial.print("Sleeping: ");
    Serial.print(minutes * 60 - timeClient.getSeconds());
    Serial.println(" seconds");
  #endif
}

void stop_wifi(){
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}