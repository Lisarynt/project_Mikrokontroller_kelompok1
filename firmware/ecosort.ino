#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// konfigurasi WiFi dan MQTT
const char* WIFI_SSID     = "NAMA_WIFI_KAMU";
const char* WIFI_PASSWORD = "PASSWORD_WIFI_KAMU";

const char* MQTT_SERVER   = "broker.hivemq.com";
const int   MQTT_PORT     = 1883;
const char* MQTT_CLIENT   = "EcoSort-ESP32-001"