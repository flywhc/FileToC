/*
 This is a demo to show how to build a Web Server for ESP8266 by using ProgmemWebRequest and files in PROGMEM.
 Run following command to generate webdata.h and webdata.c from files under webdata:
    python ..\..\file_to_c.py webdata

 by github@flywhc
 License: GPL v2.1 or later (Same as the LICENSE in ESP8266WiFi.h)
*/

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include "ProgmemWebRequest.h"
#include "webdata.h"

// change config.h with your own config
#include "config.h"

// debugger helper
#define DebugBegin(baud_rate)    Serial.begin(baud_rate);
#define DebugPrint(message)      Serial.print(message);
#define DebugPrintln(message)    Serial.println(message);

ESP8266WebServer webserver(HTTP_PORT);
bool isLEDOn = false;

void setup() {
  DebugBegin(SERIAL_BAUD_RATE);
  pinMode(LED_BUILTIN, OUTPUT);
  setLEDLight(true);
  initWiFi();
  
  // mDNS register service in LAN
  if (!MDNS.begin(HOSTNAME)) {
    DebugPrintln("Error: Cannot setup MDNS responder!");
  }
  else
  {
    MDNS.addService("http", "tcp", HTTP_PORT); 
  }

// Web server
  webserver.on("/api/toggle_led", toggleLed); // a simple web API to toggle built-in LED
  const char *ignoredDirectories[] = {"/api", "/cgi-bin"}; // dynamic web pages/APIs must be stored under /api or /cgi-bin folder
  webserver.addHandler(new ProgmemWebRequest(progmemFiles, ignoredDirectories, sizeof(ignoredDirectories)/sizeof(char*))); // handle requests to files in PROGMEM
  // do not add webserver.on() after addHandler()


  //start webserver
  webserver.begin();
  DebugPrintln("HTTP server started");
  setLEDLight(false);
}

void loop() {
  webserver.handleClient();
}

// connect WIFI
void initWiFi(){
// === setup WIFI
  WiFi.setHostname(HOSTNAME);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  DebugPrintln("Connecting WIFI");
  int wifiStatus;
  char buf[50];
  int count=1;
  while ((wifiStatus=WiFi.status()) != WL_CONNECTED) {
    delay(500);
    setLEDLight(false);
    delay(500);
    setLEDLight(true);
    switch(wifiStatus) {
      case WL_NO_SSID_AVAIL:
        DebugPrintln("No WIFI SSID available");
        break;
      case WL_WRONG_PASSWORD:
        DebugPrintln("Wrong WIFI password");
        break;
      default:
        sprintf(buf, "%d,", wifiStatus);
        DebugPrint(buf);
        if((count++ % 80) == 0) DebugPrintln("");
    }
  }

  DebugPrintln("");
  sprintf(buf, "Connected to %s as %s: %s", WiFi.SSID().c_str(), WiFi.hostname().c_str(), WiFi.localIP().toString().c_str());
  DebugPrintln(buf);
}

// set built-in LED light
void setLEDLight(bool isOn){
  digitalWrite(LED_BUILTIN, isOn?LOW:HIGH);
  isLEDOn = isOn;
}

// a web API to toggle built-in LED
void toggleLed() {
  setLEDLight(!isLEDOn);
  webserver.send_P(200, "text/plain", isLEDOn?"true":"false");
}