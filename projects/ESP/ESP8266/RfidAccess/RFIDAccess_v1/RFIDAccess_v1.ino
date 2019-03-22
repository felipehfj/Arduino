#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include "MFRC522.h"

/*
   PINOUT
   +--------------------------------+---------------+
   | WEMOS D1 ESP8266 BOARD         | CONECT TO PIN |
   +--------------------------------+---------------+
   | PIN  | FUCTION  | ESP-8266 PIN | RC522 | RELAY |
   +------+----------+--------------+-------+-------+
   | 3.3V | POWER    | 3.3V         | 3.3V  |       |
   +------+----------+--------------+-------+-------+
   | 5V   | POWER    | 5V           |       | VCC   |
   +------+----------+--------------+-------+-------+
   | GND  | GND      | GND          | GND   | GND   |
   +------+----------+--------------+-------+-------+
   | D13  | SCK      | GPIO-14      | SCK   |       |
   +------+----------+--------------+-------+       +
   | D12  | MISO     | GPIO-12      | MISO  |       |
   +------+----------+--------------+-------+       +
   | D11  | MOSI     | GPIO-13      | MOSI  |       |
   +------+----------+--------------+-------+       +
   | D10  | SS (SDA) | GPIO-15      | SDA   |       |
   +------+----------+--------------+-------+       +
   | D8   | IO       | GPIO-0       | RESET |       |
   +------+----------+--------------+-------+-------+
   | D2   | IO       | GPIO-16      |       | IN1   |
   +------+----------+--------------+-------+-------+
*/

#define RST_PIN 0 // RST-PIN for RC522 - RFID - SPI - Module GPIO-0 
#define SS_PIN  15  // SDA-PIN for RC522 - RFID - SPI - Module GPIO-15
#define RELAY_PIN 16 // RELAY-PIN in GPI0-16

const char* ssid     = "Acropole";
const char* password = "felipehfj";

const char* host = "192.168.1.7";

// Allocate the JSON document
// Use arduinojson.org/v6/assistant to compute the capacity.
StaticJsonDocument<200> doc;


int tag[4];

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "" : ".");
    Serial.print(buffer[i], DEC);
    tag[i] = buffer[i];
  }
}

void setup() {

  pinMode(RELAY_PIN, OUTPUT);
  // Initialize serial communications
  Serial.begin(115200);
  delay(10);
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}

void loop() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    delay(50);
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  // Show some details of the PICC (that is: the tag/card)
  Serial.println("RFID Tag Detected...");
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  Serial.println();
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 3000;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the request
  String url = "/api/";
  url += tag[0];
  url += ".";
  url += tag[1];
  url += ".";
  url += tag[2];
  url += ".";
  url += tag[3];

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.println(
    String("GET ") + url + " HTTP/1.1\r\n" +
    "Host: " + host + "\r\n" +
    "Connection: close\r\n\r\n");

  delay(10);

  while (client.connected()) {

    // Check HTTP status
    char status[32] = {0};
    client.readBytesUntil('\r', status, sizeof(status));
    if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
      Serial.print(F("Unexpected response: "));
      Serial.println(status);
      return;
    }

    // Skip HTTP headers
    char endOfHeaders[] = "\r\n\r\n";
    if (!client.find(endOfHeaders)) {
      Serial.println(F("Invalid response"));
      return;
    }


    // Parse JSON object
    DeserializationError error = deserializeJson(doc, client);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    // Disconnect
    client.stop();
    check();
  }

  Serial.println();
  Serial.println("closing connection");
  Serial.println();

  delay(5000);
}

void check() {
  bool authorized = doc["authorized"];
  const char * message = doc["message"];

  Serial.println(F("Response:"));
  Serial.printf("Autorizado: %s", authorized);
  Serial.printf("Mensagem: %s", message);
}
