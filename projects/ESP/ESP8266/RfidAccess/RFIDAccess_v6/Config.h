#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <ShiftLCD.h>
#include "MFRC522.h"
#include <WiFiUdp.h>
#include "pgmspace.h"
#include "time.h"

#define RST_PIN 0 // RST-PIN for RC522 - RFID - SPI - Module GPIO-0 
#define SS_PIN  15  // SDA-PIN for RC522 - RFID - SPI - Module GPIO-15
#define RELAY_PIN 16 // RELAY-PIN in GPI0-16

WiFiUDP ntpUDP;

NTPClient timeClient(ntpUDP, "a.st1.ntp.br", -10800, 60000);
uint32_t currentMillis = 0;
uint32_t previousMillis = 0;

const char* wifiName = "Acropole";
const char* wifiPass = "felipehfj";

//Web Server address to read/write from
const char *host = "192.168.1.7";

int tag[4];

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

ShiftLCD lcd(2, 5, 4);

const size_t capacity = JSON_OBJECT_SIZE(2) + 80;
DynamicJsonDocument doc(capacity);


bool idle = true;

static const char WIFI_CONNECTING[] PROGMEM = "Conectando......";
static const char WIFI_CONNECTING_OK[] PROGMEM = "  Conexao OK!   ";
static const char WIFI_CONNECTING_IP[] PROGMEM = "  Endereco IP   ";

static const char RFID_READED[] PROGMEM = "  Cartao lido   ";
static const char RFID_GET_AUTH[] PROGMEM = "Consultando.....";
static const char RFID_GET_CARD[] PROGMEM = " Aproxime a TAG ";
static const char RFID_PLEASEWAITING[] PROGMEM = "favor aguardar  ";
