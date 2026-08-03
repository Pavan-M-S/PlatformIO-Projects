#include <WiFi.h>
#include <WebServer.h>

// Wi-Fi credentials
#define WIFI_SSID "HOME Wi-Fi"
#define WIFI_PASSWORD "MSPP1234"

// Web server on port 80
WebServer server(80);

// Traffic variables
float R1, R1T, R2, R2T, R3, R3T, R4, R4T, Mode;

void handleData() {
  String html = "<html><head><title>Traffic Data</title></head><body>";
  html += "<h2>Dynamic Traffic Timer</h2>";
  html += "<p>Road 1: " + String(R1) + " | Timer: " + String(R1T) + "</p>";
  html += "<p>Road 2: " + String(R2) + " | Timer: " + String(R2T) + "</p>";
  html += "<p>Road 3: " + String(R3) + " | Timer: " + String(R3T) + "</p>";
  html += "<p>Road 4: " + String(R4) + " | Timer: " + String(R4T) + "</p>";
  html += "<p>Mode: " + String(Mode) + "</p>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

void handleEmergency() {
  for (int i = 0; i < 35; i++) {
    Serial1.println("Emergency");
    delay(300);
  }
  server.send(200, "text/plain", "Emergency triggered");
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(38400, SERIAL_8N1, 22, 21); // GPIO22=TX, GPIO21=RX

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected with IP: " + WiFi.localIP().toString());

  // Web server routes
  server.on("/data", handleData);
  server.on("/emergency", handleEmergency);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();

  if (Serial1.available()) {
    String input = Serial1.readStringUntil('\n');
    input.trim();
    Serial.println(input);

    int start = 0;
    while (start < input.length()) {
      int sep = input.indexOf(';', start);
      if (sep == -1) sep = input.length();

      String pair = input.substring(start, sep);
      int colon = pair.indexOf(':');
      if (colon != -1) {
        String key = pair.substring(0, colon);
        String value = pair.substring(colon + 1);
        float val = value.toFloat();

        if (key == "R1") R1 = val;
        else if (key == "R1T") R1T = val;
        else if (key == "R2") R2 = val;
        else if (key == "R2T") R2T = val;
        else if (key == "R3") R3 = val;
        else if (key == "R3T") R3T = val;
        else if (key == "R4") R4 = val;
        else if (key == "R4T") R4T = val;
        else if (key == "Mode") Mode = val;
      }

      start = sep + 1;
    }
  }
}