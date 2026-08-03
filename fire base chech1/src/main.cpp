#include <WiFi.h>
#include <FirebaseESP32.h>

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



void setup() {
  Serial.begin(115200);      // USB Serial (PC)

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

  // Assign Firebase 
  config.api_key = FIREBASE_PROJECT_API_KEY;
  config.database_url = FIREBASE_DATABASE_URL;

  auth.user.email = FIREBASE_EMAIL;
  auth.user.password = FIREBASE_PASSWORD;

  // Initialize Firebase
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial1.begin(9600, SERIAL_8N1, 23, 22); // UART Serial1 (Nano) on GPIO23=RX, GPIO17=TX
  Serial.println("Ready to receive from PC or Nano...");
}

void loop() {
  String input = "";

  //  from PC
  if (Serial.available()) {
    input = Serial.readStringUntil('\n');
  }

  //  from Arduino Nano
  if (Serial1.available()) {
    input = Serial1.readStringUntil('\n');
  }

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
  
          } else if (key == "Mode"){ Mode = val;
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

  
  

/*if (Firebase.setFloat(fbdo, "/R1C", R1) &&
  Firebase.setFloat(fbdo, "/R1T", R1T) &&
  Firebase.setFloat(fbdo, "/R2C", R2) &&
  Firebase.setFloat(fbdo, "/R2T", R2T) &&
  Firebase.setFloat(fbdo,"/Mode",Mode)) {
  Serial.println("Data sent to Firebase successfully!");
} else {
  Serial.println("Failed to send data to Firebase");
  Serial.println(fbdo.errorReason());
}*/
  }
}
