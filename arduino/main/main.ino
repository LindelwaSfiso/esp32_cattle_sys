// Cattle Weighting System Arduino Code
// Date: 30 October 2023
// Electrical and Electronics

// Library imports
#include <WiFi.h>
#include <HTTPClient.h>

// mfrc522 rfid headers
#include <SPI.h>
#include <MFRC522.h>

// motors headers
#include <ESP32Servo.h>

// hx711 headers
#include <Arduino.h>
#include "HX711.h"
#include "soc/rtc.h"

// hx711 pin setup
#define LOADCELL_DOUT_PIN 16
#define LOADCELL_SCK_PIN 4

#define SS_PIN 5
#define RST_PIN 0

#define OPEN_SERVO_PIN 25
#define CLOSE_SERVO_PIN 26

// Variables
#define SCALE_CALIBRATION_FACTOR 1

const char* ssid = "Lindelwa";
const char* password = "123456789";


String serverPath = "http://192.168.8.101:8000";        // Local HTTP Server
String uploadServerPath = serverPath + "/api/upload/";  // HTTP endpoint for data upload


// Initialization
WiFiClient client;  // Local WiFi Client [HTTP]
HTTPClient http;

HX711 scale;
MFRC522 rfid(SS_PIN, RST_PIN);


MFRC522::MIFARE_Key key;
// Init array that will store new NUID
byte nuidPICC[4];

// motors setup
Servo openServo, closeServo;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  // setup esp32 internal params
  rtc_cpu_freq_config_t config;
  rtc_clk_cpu_freq_get_config(&config);
  rtc_clk_cpu_freq_mhz_to_config(RTC_XTAL_FREQ_40M, &config);
  rtc_clk_cpu_freq_set_config_fast(&config);

  // setup servo motors
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  openServo.setPeriodHertz(50);   // standard 50 hz servo
  closeServo.setPeriodHertz(50);  // standard 50 hz servo

  openServo.attach(OPEN_SERVO_PIN, 900, 2100);
  closeServo.attach(CLOSE_SERVO_PIN, 900, 2100);

  openServo.write(0);
  closeServo.write(0);
  delay(1000);

  // initialize WiFi, and connect, later disable this to work with ADC pins
  WiFi.begin(ssid, password);
  Serial.print("Connecting.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("\n");


  Serial.println("Initializing the scale");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  scale.set_scale(SCALE_CALIBRATION_FACTOR);
  scale.tare();


  Serial.println("Initializing the RFID");
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println("\n");
}

void loop() {
  // put your main code here, to run repeatedly:

  Serial.print("SCALE READING:\t");
  Serial.print(scale.read_average(20));
}

/***
Function to upload weight data to our remote server
*/
void remoteServerUploadData(float weight, String rfid) {
  // Confirm that ESP-32 is still connected to the WiFi
  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("1. CONNECTING TO SERVER.....");
    http.begin(client, uploadServerPath.c_str());
    // for server authentication, if server APIs are protected
    // http.setAuthorization("SERVER_USERNAME", "SERVER_PASSWORD");

    // configure http, set content-type to JSON
    http.addHeader("Content-Type", "application/json");

    // Construct HTTP Request Payload a
    String httpRequestData = makeRequestDataJson(weight, rfid);


    // Send HTTP POST Request, and Check response code
    int httpResponseCode = http.POST(httpRequestData);

    // Check HTTP Response Code :: Success Status
    if (httpResponseCode == 200) {

      Serial.println("=> UPLOAD SUCCESSFUL");

    } else {

      // Request was not successful, Print the reason why?

      Serial.println("FAILED TO UPLOAD: SERVER ERROR. AN ERROR OCCURRED.");
      // Serial the server response
      String payload = http.getString();
      Serial.print("FAILURE REASON:");
      Serial.println(payload);
    }


    // Free up Resources
    http.end();
  }

  else {
    Serial.println("FAILED TO UPLOAD: NOT CONNECTED TO ANY NETWORK");
  }
}


String makeRequestDataJson(float weight, String rfid) {
  String requestData = "{\"rfid\": \"";
  requestData.concat(rfid);
  requestData.concat("\", ");
  requestData.concat("\"weight\": \"");
  requestData.concat(weight);
  requestData.concat("\"}");
  return requestData;
}


String readScannedCard() {
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if (!rfid.PICC_IsNewCardPresent())
    return "";

  // Verify if the NUID has been readed
  if (!rfid.PICC_ReadCardSerial())
    return "";

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return "";
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] || rfid.uid.uidByte[1] != nuidPICC[1] || rfid.uid.uidByte[2] != nuidPICC[2] || rfid.uid.uidByte[3] != nuidPICC[3]) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  } else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}



/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte* buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(' ');
    Serial.print(buffer[i], DEC);
  }
}
