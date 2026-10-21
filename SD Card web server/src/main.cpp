#include <Arduino.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <WebServer.h>
#include <ESPWebDAV.h>

// Wi-Fi Credentials
const char *ssid = "narzo 50A";
const char *password = "MSPP9347";

#define SD_CS_PIN 5 // Standard CS pin for 6-pin SPI SD Module

// Server instances
WebServer server(8000);      // Phone/Browser interface on port 8000
WiFiServer davServer(10000); // Windows WebDAV network drive on port 10000
ESPWebDAV dav;

// Helper: Maps file extensions to MIME types so the browser can display them
String getContentType(String filename)
{
  filename.toLowerCase();

  if (filename.endsWith(".htm") || filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "application/javascript";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg"))
    return "image/jpeg";
  else if (filename.endsWith(".pdf"))
    return "application/pdf";
  else if (filename.endsWith(".mp4"))
    return "video/mp4";
  else if (filename.endsWith(".mp3"))
    return "audio/mpeg";
  else if (filename.endsWith(".txt") || filename.endsWith(".log") || filename.endsWith(".json") || filename.endsWith(".cpp") || filename.endsWith(".h"))
    return "text/plain";

  return "application/octet-stream"; // Default fallback (triggers download)
}

// Python-style file browser handler (supports infinite subdirectory navigation)
void handleFileBrowser()
{
  String path = server.uri();
  if (path == "")
    path = "/";

  // Attempt to open file or directory
  File entry = SD.open(path);
  if (!entry)
  {
    server.send(404, "text/plain", "404: File or Folder Not Found");
    return;
  }

  // If path is a Directory -> List contents (Python HTTP Server style)
  if (entry.isDirectory())
  {
    String html = "<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:monospace; padding:15px; background:#f4f4f4;} a{text-decoration:none; color:#0066cc;} ul{list-style-type:none; padding-left:0;} li{padding:4px 0;}</style>";
    html += "</head><body>";
    html += "<h2>Directory listing for " + path + "</h2><hr><ul>";

    // Parent directory navigation link
    if (path != "/")
    {
      String parentPath = path;
      if (parentPath.endsWith("/"))
        parentPath.remove(parentPath.length() - 1);
      int lastSlash = parentPath.lastIndexOf('/');
      parentPath = parentPath.substring(0, lastSlash + 1);
      if (parentPath == "")
        parentPath = "/";

      html += "<li>📁 <a href='" + parentPath + "'>[ Parent Directory ]</a></li><br>";
    }

    File file = entry.openNextFile();
    while (file)
    {
      String fileName = String(file.name());

      // Clean up leading slash artifacts if needed
      if (fileName.startsWith("/"))
      {
        fileName = fileName.substring(1);
      }

      // Construct absolute target URI for links
      String itemUri = path;
      if (!itemUri.endsWith("/"))
        itemUri += "/";
      itemUri += fileName;

      html += "<li>";
      if (file.isDirectory())
      {
        html += "📁 <a href='" + itemUri + "'><b>" + fileName + "/</b></a>";
      }
      else
      {
        html += "📄 <a href='" + itemUri + "' target='_blank'>" + fileName + "</a>";
        html += " <small>(" + String(file.size() / 1024) + " KB)</small>";
      }
      html += "</li>";

      file.close();
      file = entry.openNextFile();
    }
    html += "</ul><hr></body></html>";

    server.send(200, "text/html", html);
  }
  // If path is a File -> Stream file contents to browser
  else
  {
    String contentType = getContentType(path);
    server.streamFile(entry, contentType);
  }

  entry.close();
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // Connect to narzo 50A Hotspot
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());


// Initialize SD Card Module with 25 MHz SPI speed for faster image reads
  if (!SD.begin(SD_CS_PIN, SPI)) {
    Serial.println("SD Card Mount Failed! Check wiring and power supply.");
    return;
  }
  Serial.println("SD Card Mounted Successfully at 10 MHz.");

  // Catch-all route to handle both files and nested directories
  server.onNotFound(handleFileBrowser);

  server.begin();
  Serial.println("HTTP Browser Server started on Port 8000");

  // Configure WebDAV Server for Windows Explorer
  davServer.begin();
  dav.begin(&davServer, &SD);
  Serial.println("WebDAV Server started on Port 10000");
}

void loop()
{
  // Service web browser requests
  server.handleClient();

  // Service WebDAV requests
  dav.handleClient();
}