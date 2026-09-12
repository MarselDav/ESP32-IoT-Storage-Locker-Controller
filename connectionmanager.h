#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <Arduino_JSON.h>
#include <TimeLib.h>
#include <Arduino.h>
#include <HTTPClient.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

#define SSID            "SSID"
#define PASSWORD        "PASSWORD"
#define MQTT_BROKER_URL "MQTT_BROKER_URL"
#define MQTT_USERNAME   "esp32"
#define MQTT_PASSWORD   "MQTT_PASSWORD"
#define MQTT_PORT       8883

#define INPUT_TOPIC  "esp32/in"
#define OUTPUT_TOPIC "esp32/out"


void callback(char* topic, byte* payload, unsigned int length);

class ConnectionManager
{
public:
  ConnectionManager();

  void setup();
  void connectWIFI();
  void reconnect();
  void setCurrentDateTime();

  void sendJSON(String impactState);
  bool isConnected();
  void update();

  bool isWifiConnect();
  bool isMqttConnect();

private:
  WiFiClientSecure secureClient;
  PubSubClient client;

  WiFiUDP ntpUDP;
  NTPClient timeClient;

  bool startConnectingWifi = false;
};

#endif