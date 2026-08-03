#include <WiFi.h>
#include <FirebaseESP32.h>
#include <SPI.h>
#include <MFRC522.h>



//  Firebase project API Key
#define FIREBASE_PROJECT_API_KEY "AIzaSyDAtbzOawekOm8xMliLVjuwJ-U_N9vfRm8"

//  Firebase Database URL
#define FIREBASE_DATABASE_URL "https://new-one-eef62-default-rtdb.firebaseio.com/"

//  Firebase Authentication 
#define FIREBASE_EMAIL "esp32@espres.com"
#define FIREBASE_PASSWORD "Pavan#310512"

//  WiFi 
#define WIFI_SSID "HOME Wi-Fi"
#define WIFI_PASSWORD "MSPP1234"




FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Variables to store  values
float R1;
float R1T;
float R2;
float R2T;
float Mode;
// Define SPI pins for ESP32
#define RST_PIN 13     // RST to GPIO4
#define SS_PIN 12      // SDA to GPIO5
#define SCK_PIN 14
#define MISO_PIN 27
#define MOSI_PIN 26
int relay =21;
int red1 = 22;
int red2 = 23;
int green1 = 5;
int green2 = 15;
int oran = 28;
int yel = 33;
int sw1 = 17;
int sw2 = 4;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
byte authorizedUID[] = {0x37, 0xe0, 0x96, 0x04};
bool authorized = false; // Declare globally




void setup() {
  pinMode(relay,OUTPUT);
  pinMode(yel,OUTPUT);
  pinMode(oran,OUTPUT);
  pinMode(sw1,INPUT);
  pinMode(sw2,INPUT);

  Serial.begin(115200);      // USB Serial (PC)

 WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  SPI.begin(SCK_PIN, MISO_PIN, MOSI_PIN, SS_PIN); // Custom SPI init for ESP32
  mfrc522.PCD_Init();

  // Assign Firebase 
  config.api_key = FIREBASE_PROJECT_API_KEY;
  config.database_url = FIREBASE_DATABASE_URL;

  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial1.begin(115200, SERIAL_8N1, 25, 19); // UART Serial1 (Nano) on GPIO33=RX, GPIO32=TX
  Serial.println("Ready to receive from PC or Nano...");

  
  mfrc522.PCD_DumpVersionToSerial();
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
}
void led(){
  digitalWrite(relay,LOW);
  if (digitalRead(sw1) == HIGH) {
  digitalWrite(red1, HIGH);     // Red ON
  digitalWrite(green1, LOW);    // Green OFF
} else {
  digitalWrite(red1, LOW);      // Red OFF
  digitalWrite(green1, HIGH);   // Green ON
}
if (digitalRead(sw2) == HIGH) {
  digitalWrite(red2, HIGH);     // Red ON
  digitalWrite(green2, LOW);    // Green OFF
} else {
  digitalWrite(red2, LOW);      // Red OFF
  digitalWrite(green2, HIGH);   // Green ON
}
}
void loop() {

if(authorized){
led();
	} 




  if ( mfrc522.PICC_IsNewCardPresent() ) {
		
    if (mfrc522.PICC_ReadCardSerial()){
    String id = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
		id.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
		id.concat(String(mfrc522.uid.uidByte[i], HEX));
	
  } 
	
	Serial.println("________________________________________break________________________________________");
	Serial.println("Tag Uid is");
	Serial.println(id);
	
authorized = true;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != authorizedUID[i]) {
      authorized = false;
      break;
    }
  }


   

	if(authorized){
		Serial.println("Access granted!!!!");
		Serial1.println("Manual");
    digitalWrite(oran, HIGH);

    Firebase.setFloat(fbdo,"/Mode", 2);
    delay(1500);
    digitalWrite(oran,LOW);
   led();
    } else {
Serial.println("Access declined");
digitalWrite(yel, HIGH);

Firebase.setFloat(fbdo,"/Mode", 0);
    digitalWrite(yel,LOW);
	}

	} 
	mfrc522.PICC_HaltA();

 }
  
  String input = "";

  //  from PC
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
  }
  if (Firebase.getFloat(fbdo, "/Set") == 1) {
  Serial1.println("Emergency");
  
}

  //  from Arduino Nano
  if (Serial1.available()) {
    input = Serial1.readStringUntil('\n');
  

  if (input.length() > 0) {
    input.trim(); // Remove whitespace

    int start = 0;
    while (start < input.length()) {
      int sep = input.indexOf(';', start);
      if (sep == -1) sep = input.length();

      String pair = input.substring(start, sep);
      int colon = pair.indexOf(':');
      if (colon != -1) {
        String key = pair.substring(0, colon);
        String value = pair.substring(colon + 1);
        int val = value.toInt();

        if (key == "R1") {R1 = val;
        }
       else if (key == "R1T") {
            R1T = val;
       }

      else if (key == "R2") {R2 = val;
      }
      else if (key == "R2T") {
       R2T = val;
  
          } else if (key == "Mode"){Mode = val;
          }
      }

      start = sep + 1;
    }

    
    

    // Print output inside loop
    Serial.println("------------------------");
    Serial.print("Road 1 = "); Serial.println(R1);
    Serial.println("  "); Serial.println(R1T);
    Serial.println("------------------------");
    Serial.print("Road 2 = "); Serial.println(R2);
    Serial.println("  "); Serial.println(R2T);
    Serial.println("------------------------");

  
  

if (Firebase.setFloat(fbdo, "/R1C", R1) &&
  Firebase.setFloat(fbdo, "/R1T", R1T) &&
  Firebase.setFloat(fbdo, "/R2C", R2) &&
  Firebase.setFloat(fbdo, "/R2T", R2T) &&
  Firebase.setFloat(fbdo,"/Mode", Mode)) {
  Serial.println("Data sent to Firebase successfully!");
} else {
  Serial.println("Failed to send data to Firebase");
  Serial.println(fbdo.errorReason());
}
  }
  }
}
