#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <AES.h>
#include <AESLib.h>
#include <AES_config.h>
#include <base64.h>
#include <Arduino.h>

int lcdColumns = 16;
int lcdRows = 2;
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "your ssid"
#define WIFI_PASSWORD "your pass"

// Insert Firebase project API Key
#define API_KEY "AIzaSyBofNxrCZYMJ4ATDn8TDty_GktYYdxkqd8"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "testiot-kma-default-rtdb.firebaseio.com"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int intValue;
float floatValue;
bool signupOK = false;

String text_decrypt;
// we need to install AESLib and ESP8266 additional board.

AES aes;
byte cipher[1000];
char b64[1000];

// msg: message need to be encrypted.
// key_str: secrete key, 16 bytes
// iv_str:  initial vector, 16 bytes
void do_encrypt(String msg, String key_str, String iv_str) {

  byte iv[16];
  // copy the iv_str content to the array.
  memcpy(iv, (byte *) iv_str.c_str(), 16);

  // use base64 encoder to encode the message content. It is optional step.
  int blen = base64_encode(b64, (char *)msg.c_str(), msg.length());

  // calculate the output size:
  aes.calc_size_n_pad(blen);
  // custom padding, in this case, we use zero padding:
  int len = aes.get_size();
  byte plain_p[len];
  for (int i = 0; i < blen; ++i) plain_p[i] = b64[i];
  for (int i = blen; i < len; ++i) plain_p[i] = '\0';

  // do AES-128-CBC encryption:
  int blocks = len / 16;
  aes.set_key ((byte *)key_str.c_str(), 16) ;
  aes.cbc_encrypt (plain_p, cipher, blocks, iv);

  // use base64 encoder to encode the encrypted data:
  base64_encode(b64, (char *)cipher, len);
  Serial.println("Encrypted Data output: " + String((char *)b64));
//  Serial.printf("Trang thai den 1... %s\n", Firebase.RTDB.setString(&fbdo, F("/AES_algorithm/byte1"), String((char *)b64)) ? "ok" : fbdo.errorReason().c_str());

}


String do_decrypt(String msg, String key_str, String iv_str) {

  byte iv[16];
  // copy the iv_str content to the array.
  memcpy(iv, (byte *) iv_str.c_str(), 16);

  // use base64 encoder to encode the message content. It is optional step.
  int blen = base64_encode(b64, (char *)msg.c_str(), msg.length());

  // calculate the output size:
  aes.calc_size_n_pad(blen);
  // custom padding, in this case, we use zero padding:
  int len = aes.get_size();
  byte plain_p[len];
  for (int i = 0; i < blen; ++i) plain_p[i] = b64[i];
  for (int i = blen; i < len; ++i) plain_p[i] = '\0';

  // do AES-128-CBC encryption:
  int blocks = len / 16;
  aes.set_key ((byte *)key_str.c_str(), 16) ;
  aes.cbc_decrypt (plain_p, cipher, blocks, iv);

  // use base64 encoder to encode the encrypted data:
  base64_decode(b64, (char *)cipher, len);
  //  return String((char *)b64);
  return msg;

}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  lcd.begin();
  // turn on LCD backlight
  lcd.backlight();
  Serial.println();
  Serial.println();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1500 || sendDataPrevMillis == 0)) {
    lcd.setCursor(0, 0);
    // print message
    

    sendDataPrevMillis = millis();
    String key_str = "aaaaaaaaaaaaaaaa"; // 16 bytes
    String iv_str = "aaaaaaaaaaaaaaaa"; //16 bytes
    String cipher_text = Firebase.RTDB.getString(&fbdo, F("/AES_algorithm/ciphertext")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str();
    lcd.print(cipher_text);
    text_decrypt = Firebase.RTDB.getString(&fbdo, F("/users/admin/userName")) ? fbdo.to<const char *>() : fbdo.errorReason().c_str();
    //    String text_decrypt = do_decrypt(led_state,key_str,iv_str);
    //    Serial.println(cipher_text);
    lcd.setCursor(0, 1);
    lcd.print(text_decrypt);
    Serial.println(text_decrypt);
    
    
    delay(1000);

  }
  lcd.clear();
}
