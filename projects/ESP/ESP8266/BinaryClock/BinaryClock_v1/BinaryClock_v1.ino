#include <ESP8266WiFi.h> //For ESP8266
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <NTPClient.h>

char * hostName = "ESP-BINCLOCK";

#ifndef STASSID
#define STASSID "Acropole"
#define STAPSK  "felipehfj"
#endif

const char* ssid     = STASSID;
const char* password = STAPSK;

//To make Arduino software autodetect OTA device
WiFiServer TelnetServer(8266);

WiFiClient cliente;
WiFiUDP Udp;

int16_t utc = -3; //UTC -3:00 Brazil
uint32_t currentMillis = 0;
uint32_t previousMillis = 0;
NTPClient timeClient(Udp, "a.st1.ntp.br", utc * 3600, 60000);

uint8_t hControl = 4;
uint8_t mControl = 5;

uint8_t dataPin  = 14;
uint8_t latchPin = 12;
uint8_t clockPin = 13;

void setupWifi() {
  wifi_station_set_hostname(hostName);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  logTime();
  Serial.println(F("Conectado a rede WiFi :-)"));
  logTime();
  Serial.print(F("IP: "));
  Serial.println(WiFi.localIP()); 
  WiFi.printDiag(Serial);
}

void setupOTA() {
  ArduinoOTA.setHostname(hostName);
  ArduinoOTA.begin();
}

void logTime() {
  Serial.print('[');
  Serial.print(millis() / 1000);
  Serial.print("] ");
}

int dec2bcd(int dec) {
  return (dec / 10) * 16 + (dec % 10);
}

void showHour(int hh, int mm) {
  
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, hh);
  digitalWrite(latchPin, HIGH);
  digitalWrite(hControl, HIGH);
  digitalWrite(mControl, LOW);
  delayMicroseconds(250);
  
  digitalWrite(latchPin, LOW);
  shiftOut(dataPin, clockPin, MSBFIRST, mm);
  digitalWrite(latchPin, HIGH);
  digitalWrite(hControl, LOW);
  digitalWrite(mControl, HIGH);
  delayMicroseconds(250);
}

void checkOST(void) {
  currentMillis = millis();//Tempo atual em ms
  //Lógica de verificação do tempo
  if (currentMillis - previousMillis > 10000) {
    previousMillis = currentMillis;    // Salva o tempo atual
    //Serial.printf("Data-hora: %d: ", timeClient.getEpochTime());
    Serial.printf("Hora: %s \n", timeClient.getFormattedTime().c_str());
  }
}

void setup() {
  // put your setup code here, to run once:
  pinMode(hControl, OUTPUT);
  pinMode(mControl, OUTPUT);
  pinMode(dataPin, OUTPUT);
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  
  /* Serial */
  Serial.begin(115200);

  Serial.setDebugOutput(true);
  
  /* Wifi */
  setupWifi();

  if (MDNS.begin(hostName)) {
    logTime();
    Serial.println("MDNS responder iniciado");
  }

  /* OTA */
  setupOTA();

  /* NTP Time client */
  timeClient.begin();
  timeClient.update();

}

void loop() {
  // put your main code here, to run repeatedly:
  ArduinoOTA.handle();
  timeClient.update();
  checkOST();

  showHour(dec2bcd(timeClient.getHours()), dec2bcd(timeClient.getMinutes()));
}
