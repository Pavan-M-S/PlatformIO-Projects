#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <WebServer.h>
#include <ESPWebDAV.h>

// Replace with your actual network details
const char* ssid = "narzo 50A";
const char* password = "MSPP9347";

#define SD_CS_PIN 5 // The CS pin for your 6-pin SD Card module

// 1. Create the Web Server for Phones/Browsers (Default Port 8000)
WebServer server(8000);

// 2. Create the WebDAV Server for Windows (Custom Port 10000)
ESPWebDAV dav;
WiFiServer davServer(10000);

// Helper function to tell the browser how to display the file
String getContentType(String filename) {
  filename.toLowerCase(); 
  
  if (filename.endsWith(".htm") || filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
  else if (filename.endsWith(".pdf")) return "application/pdf";
  else if (filename.endsWith(".mp4")) return "video/mp4";
  else if (filename.endsWith(".txt") || filename.endsWith(".log") || filename.endsWith(".json") || filename.endsWith(".cpp") || filename.endsWith(".h")) return "text/plain";
  
  // If it's a file type the browser can't read, default to downloading it
  return "application/octet-stream"; 
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi
  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize the SD Card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card Mount Failed! Check wiring.");
    return;
  }
  Serial.println("SD Card Mounted Successfully.");

  // Configure the Phone/Browser HTML Interface
  server.on("/", HTTP_GET, []() {
    String html = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1'></head>";
    html += "<body style='font-family:sans-serif; padding: 20px;'><h2>ESP32 SD Card</h2><ul>";
    
    File root = SD.open("/");
    File file = root.openNextFile();
    while (file) {
      html += "<li style='margin-bottom:10px;'>";
      // target="_blank" ensures it opens in a new tab
      html += "<a href=\"/" + String(file.name()) + "\" target=\"_blank\">" + String(file.name()) + "</a>";
      html += " <small>(" + String(file.size() / 1024) + " KB)</small></li>";
      file = root.openNextFile();
    }
    html += "</ul></body></html>";
    server.send(200, "text/html", html);
  });

  // Handle viewing the file when clicked
  server.onNotFound([]() {
    String path = server.uri();
    if (SD.exists(path)) {
      File file = SD.open(path, FILE_READ);
      String contentType = getContentType(path); 
      server.streamFile(file, contentType);
      file.close();
    } else {
      server.send(404, "text/plain", "File not found on SD Card.");
    }
  });

  server.begin();
  Serial.println("HTTP Browser Server started on Port 80");

  // Configure the WebDAV Server for Windows
  dav.begin(&SD);
  davServer.begin();
  Serial.println("WebDAV Server started on Port 10000");
}

void loop() {
  // Listen for Phone/Browser requests
  server.handleClient();

  // Listen for Windows WebDAV requests
  WiFiClient client = davServer.accept();
  if (client) {
    dav.handleClient(&client);
  }
}