// Cattle Weighting System Arduino Code
// Date: 30 October 2023
// Electrical and Electronics

// Library imports
#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>

// mfrc522 rfid headers
#include <SPI.h>
#include <MFRC522.h>

// motors headers
#include <ESP32Servo.h>

// hx711 headers
#include "HX711.h"

// hx711 pin setup
#define LOADCELL_DOUT_PIN 16
#define LOADCELL_SCK_PIN 4

// mfrc522 rfid pin setup
#define SS_PIN 5
#define RST_PIN 21

// servo motors pins
#define OPEN_SERVO_PIN 25
#define CLOSE_SERVO_PIN 26


// push buttons pins
#define OPEN_BUTTON_PIN 17
#define CLOSE_BUTTON_PIN 27
#define PROCESS_BUTTON_PIN 13

// hx711 setup variables
#define SCALE_CALIBRATION_FACTOR 110.321114
#define SCALE_OFFSET_FACTOR 458042

const char* ssid = "Lindelwa";
const char* password = "123456789";


String serverName = "http://192.168.8.101:8000";        // Local HTTP Server
String uploadServerPath = serverName + "/api/upload/";  // HTTP endpoint for data upload


// Initialization
WiFiClient client;  // Local WiFi Client [HTTP]
HTTPClient http;

HX711 scale;
MFRC522 rfid(SS_PIN, RST_PIN);


MFRC522::MIFARE_Key key;

// motors setup
Servo openServo, closeServo;

// push button control state
unsigned long lastOpenDebounceTime = 0;
unsigned long lastCloseDebounceTime = 0;
unsigned long lastProcessDebounceTime = 0;
int lastOpenButtonState = HIGH;
int lastCloseButtonState = HIGH;
int lastProcessButtonState = HIGH;

// whether it is permited to open or close gates,
// this avoids opening a gate while we are weighing another cow
bool shouldOpenGates = true;

bool isOpenGateOpen = false;
bool isCloseGateOpen = false;
bool isCowPresentForProcessing = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.print("\nSetting up motors.......\n");
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

  Serial.print("\nSetting up push buttons.......\n");
  pinMode(OPEN_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CLOSE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PROCESS_BUTTON_PIN, INPUT_PULLUP);

  // initialize WiFi, and connect, later disable this to work with ADC pins
  WiFi.begin(ssid, password);
  Serial.print("Connecting..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("\n");

  Serial.print("\nInitializing HX711 scale.......\n");
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  scale.set_offset(SCALE_OFFSET_FACTOR);
  scale.set_scale(SCALE_CALIBRATION_FACTOR);
  scale.tare();


  Serial.print("\nInitializing mrf522 rfid reader......\n");
  SPI.begin();      // Init SPI bus
  rfid.PCD_Init();  // Init MFRC522

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println("\n");

  Serial.print("\nDONE SETUP, ENTERING LOOP PROCESS......\n");
}

void loop() {
  // put your main code here, to run repeatedly:

  // check if the open button is pressed
  if (buttonPressed(OPEN_BUTTON_PIN, lastOpenButtonState, lastOpenDebounceTime) && shouldOpenGates) {
    // toogle the state of the OPENING gate
    // if it was previously open, close it, visa versa
    operateOpeningMotor(!isOpenGateOpen);

    // if we just closed the opening gate
    // we assume there is a cow in the scale therefore change the flag isCowPresentForProcessing to true
    isCowPresentForProcessing = !isOpenGateOpen;
  }


  // check if the close button is pressed
  if (buttonPressed(CLOSE_BUTTON_PIN, lastCloseButtonState, lastCloseDebounceTime) && shouldOpenGates) {
    // toogle the state of the closing gate
    // if it was previously open, close it, visa versa
    operateClosingMotor(!isCloseGateOpen);
    // if the gate is open, we assume there is no cow for processing, visa versa
    isCowPresentForProcessing = !isCloseGateOpen;
  }


  // we then check if the processing button is pressed
  // we also check if the isCowPresentForProcessing is true before processing the cow
  bool processButtonPressed = buttonPressed(PROCESS_BUTTON_PIN, lastProcessButtonState, lastProcessDebounceTime);
  if (processButtonPressed && isCowPresentForProcessing) {
    // process the cow
    readValuesAndUpload();
  }
}


/** 
* Function to read rfid and weight readings and upload to the server
* attempt to wait 3 seconds for cow to get to position and align with the rfid
* and position itself on the scale then
* attempt to read the rfid, and the weight
*/
void readValuesAndUpload() {
  Serial.println("Delaying 3s for cow to settle down on the sacale and align with rfid correctly.......");
  delay(3000);

  // then read the rfid, and read the weight
  String readTag = readScannedCard();

  // if the tag is blank, we did not read anything, there is something wrong with the rfid
  // attempt to try notify farmer to position the cow correctly
  if (readTag.length() == 0) {
    Serial.println("Failed to read rfid, the cow is not positioned corrently, very far from the reader........");
    return;  // fatal exit!
  }

  // get cow reading
  float weight = getScaleReading();

  // then upload
  bool isUploadSuccessful = remoteServerUploadData(weight, readTag);

  if (isUploadSuccessful) {
    // server upload was successful,
    Serial.println("Done Upload.... Open the closing gate.......");
  } else {
    // failed
    Serial.println("Failed to upload, try again.......");
  }
}

/***
Function to upload weight data to our remote server, 
returns true if the upload was successfull
*/
bool remoteServerUploadData(float weight, String rfid) {
  bool isSuccessful = false;
  Serial.println("Uploading data to the server.....");

  // Confirm that ESP-32 is still connected to the WiFi
  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("Connecting to server: " + serverName);
    http.begin(client, uploadServerPath.c_str());

    // for server authentication, if server APIs are protected
    // http.setAuthorization("SERVER_USERNAME", "SERVER_PASSWORD");

    // configure http, set content-type to JSON
    http.addHeader("Content-Type", "application/json");

    // Construct HTTP Request Payload with rfid and weight
    String requestData = "{\"rfid\": \"";
    requestData.concat(rfid);
    requestData.concat("\", ");
    requestData.concat("\"weight\": \"");
    requestData.concat(weight);
    requestData.concat("\"}");

    // Send HTTP POST Request, and Check response code
    int httpResponseCode = http.POST(requestData);

    // Check HTTP Response Code :: Success Status
    if (httpResponseCode == 200) {

      Serial.println("=> UPLOAD SUCCESSFUL");
      isSuccessful = true;

    } else {
      // Request was not successful, Print the reason why?
      Serial.println("FAILED TO UPLOAD: SERVER ERROR. AN ERROR OCCURRED: ");
      Serial.println(http.getString() + "\n");
    }


    // Free up Resources
    http.end();
  }

  else {
    Serial.println("FAILED TO UPLOAD: NOT CONNECTED TO ANY NETWORK!!!!");
  }

  return isSuccessful;
}


float getScaleReading() {
  Serial.println("Getting scale reading.....");
  return scale.get_units(10);
}


/**
* function to read a scanned rfid, return an empty string if there is no card or if we can't read the card
*/
String readScannedCard() {
  Serial.println("Getting scanned rfid card.....\n");

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

  Serial.println(F("A card has been detected."));
  String readNUID = getHexString(rfid.uid.uidByte, rfid.uid.size);
  Serial.println(F("The NUID tag is:"));
  Serial.print(F("In hex: "));
  Serial.println(readNUID);

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();

  // return the read nuid
  return readNUID;
}


/**
* Function to check if a button is pressed, note the button state is passed by reference 
*/
bool buttonPressed(int pin, int& lastButtonState, unsigned long& lastDebounceTime) {
  int buttonState = digitalRead(pin);
  if (buttonState != lastButtonState) {
    lastButtonState = buttonState;
    if (lastButtonState == LOW) {
      delay(500);
      return true;
    }
  }
  return false;
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


/**
* Helper function to convert the byte array to a hex value which is a string
*/
String getHexString(byte* buffer, byte bufferSize) {
  String hexString = "";
  for (byte i = 0; i < bufferSize; i++) {
    if (buffer[i] < 0x10) {
      hexString += "0";
    }
    char hexChar[3];                    // Buffer to store the hex representation
    sprintf(hexChar, "%X", buffer[i]);  // Convert byte to hex string
    hexString += hexChar;
  }
  return hexString;
}

void operateOpeningMotor(bool shouldOpen) {
  if (shouldOpen) {
    for (int pos = 0; pos <= 170; pos += 1) {  // goes from 0 degrees to 180 degrees
      openServo.write(pos);                    // tell servo to go to position in variable 'pos'
      delay(15);                               // waits 15ms for the servo to reach the position
    }
    Serial.println("Opening gate status: OPEN\n");
  } else {
    for (int pos = 170; pos >= 0; pos -= 1) {  // goes from 180 degrees to 0 degrees
      openServo.write(pos);                    // tell servo to go to position in variable 'pos'
      delay(15);                               // waits 15ms for the servo to reach the position
    }
    Serial.println("Opening gate status: CLOSED\n");
  }

  // update the state of the open servo motor
  isOpenGateOpen = shouldOpen;
}

void operateClosingMotor(bool shouldOpen) {
  if (shouldOpen) {
    for (int pos = 0; pos <= 170; pos += 1) {  // goes from 0 degrees to 180 degrees
      closeServo.write(pos);                   // tell servo to go to position in variable 'pos'
      delay(15);                               // waits 15ms for the servo to reach the position
    }
    Serial.println("Closing gate status: OPEN\n");

  } else {
    for (int pos = 170; pos >= 0; pos -= 1) {  // goes from 180 degrees to 0 degrees
      closeServo.write(pos);                   // tell servo to go to position in variable 'pos'
      delay(15);                               // waits 15ms for the servo to reach the position
    }
    Serial.println("Closing gate status: CLOSED\n");
  }

  // update the state of the close servo motor
  isCloseGateOpen = shouldOpen;
}
