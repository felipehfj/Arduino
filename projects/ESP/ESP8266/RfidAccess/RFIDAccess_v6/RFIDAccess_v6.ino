#include "Config.h"

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? "" : ".");
    Serial.print(buffer[i], DEC);
    tag[i] = buffer[i];
  }
}

void checkOST(void) {
  currentMillis = millis();//Tempo atual em ms
  //Lógica de verificação do tempo
  if (currentMillis - previousMillis > 1000 && idle) {
    previousMillis = currentMillis;    // Salva o tempo atual
    Serial.println(timeClient.getFormattedDate());

    lcd.setCursor(0, 0);
    lcd.print(timeClient.getDayOfMonth() < 10 ? String("0") + timeClient.getDayOfMonth() : timeClient.getDayOfMonth());
    lcd.print('/');
    lcd.print(timeClient.getMonth() < 10 ? String("0") + timeClient.getMonth() : timeClient.getMonth());
    lcd.print(" - ");
    lcd.print(timeClient.getFormattedTime());
    lcd.setCursor(0, 1);
    lcd.print(FPSTR(RFID_GET_CARD));
  }
}

void setup() {
  pinMode(RELAY_PIN, OUTPUT);
  Serial.begin(115200);
  delay(10);
  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  lcd.begin(16, 2);
  Serial.println();

  Serial.print(FPSTR(WIFI_CONNECTING));
  Serial.println(wifiName);

  lcd.setCursor(0, 0);
  lcd.print(FPSTR(WIFI_CONNECTING));
  lcd.setCursor(0, 1);
  lcd.print(wifiName);

  WiFi.begin(wifiName, wifiPass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println(FPSTR(WIFI_CONNECTING_OK));
  Serial.println(FPSTR(WIFI_CONNECTING_IP));
  Serial.println(WiFi.localIP());   //You can get IP address assigned to ESP

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(FPSTR(WIFI_CONNECTING_OK));
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());

  timeClient.begin();
  timeClient.update();
}

void loop() {
  checkOST();
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
  Serial.println(FPSTR(RFID_READED));
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();

  // We now create a URI for the request
  String url = "/api/cards/auth/";
  url += tag[0];
  url += ".";
  url += tag[1];
  url += ".";
  url += tag[2];
  url += ".";
  url += tag[3];

  Serial.println();
  Serial.print(FPSTR(RFID_GET_AUTH));
  Serial.println(url.c_str());  

  HTTPClient http;    //Declare object of class HTTPClient

  String path = String(host) + ":3000" + url;

  Serial.print("Request Link:");
  Serial.println(path.c_str());

  lcd.setCursor(0, 0);
  lcd.print(FPSTR(RFID_GET_AUTH));
  lcd.setCursor(0, 1);
  lcd.print(FPSTR(RFID_PLEASEWAITING));
  
  http.begin(host, 3000, url.c_str());     //Specify request destination

  int httpCode = http.GET();            //Send the request
  String payload = http.getString();    //Get the response payload from server

  Serial.print("Response Code:"); //200 is OK
  Serial.println(httpCode);   //Print HTTP return code

  Serial.print("Returned data from Server:");
  Serial.println(payload);    //Print request response payload

  if (httpCode == 200)
  {
    // Parse JSON object
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }

    // Extract values
    Serial.println(F("Response:"));
    Serial.println(doc["authorized"].as<bool>());
    Serial.println(doc["message"].as<char*>());

    lcd.clear();
    if (doc["authorized"].as<bool>()) {
      digitalWrite(RELAY_PIN, HIGH);
      lcd.setCursor(0, 0);
      lcd.print(" Seja bem-vindo ");
      lcd.setCursor(0, 1);
      lcd.print(doc["message"].as<char*>());      
    }
    else
    {
      lcd.setCursor(0, 0);
      lcd.print(" Acesso negado  ");
      lcd.setCursor(0, 1);
      lcd.print(doc["message"].as<char*>()); 
    }   
    delay(5000);
    digitalWrite(RELAY_PIN, LOW);
  }
  else
  {
    Serial.println("Error in response");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Erro comunicacao");
    lcd.setCursor(0, 1);
    lcd.print("  com servidor  ");
    delay(5000);
  }

  http.end();  //Close connection
  lcd.clear();
  delay(10);
}
