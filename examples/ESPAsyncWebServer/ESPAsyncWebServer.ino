//
/*
 This is a demo to show how to build a Web Server for ESP8266/ESP32 by using ProgmemWebRequest and files in PROGMEM.
 Run following command to generate webdata.h and webdata.c from files under webdata:
    python ..\..\file_to_c.py webdata

 by github@flywhc
 License: GPL v2.1 or later (Same as the LICENSE in ESPAsyncWebServer.h)
*/

#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESP8266mDNS.h>
#endif
#include <ESPAsyncWebServer.h>
#include "ProgmemAsyncWebHandler.h"
#include "webdata.h"

// change config.h with your own config
#include "config.h"

// debugger helper
#define DebugBegin(baud_rate)    Serial.begin(baud_rate);
#define DebugPrint(message)      Serial.print(message);
#define DebugPrintln(message)    Serial.println(message);

AsyncWebServer server(80);
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
  const char *ignoredDirectories[] = {"/api", "/cgi-bin"}; // dynamic web pages/APIs must be stored under /api or /cgi-bin folder
  server.addHandler(new ProgmemAsyncWebHandler(progmemFiles, ignoredDirectories, sizeof(ignoredDirectories)/sizeof(char*))); // handle requests to files in PROGMEM

  /**
   * The ESPAsyncWebServer library has a limitation where it only natively supports 
   * defining routes at the top-level of the API path, such as 'http://host/api'. 
   * It does not inherently facilitate routing to deeper levels, like 
   * 'http://host/api/func', where 'func' is an additional segment specifying a 
   * particular function or resource.
   * Here is a workaround to enable routing to deeper levels.
   */  
    server.on("/api", HTTP_POST, [](AsyncWebServerRequest *request){
      int i = request->url().indexOf('/', 1);
      String func;
      if (i > 0) {
        func = request->url().substring(i + 1);
        i = func.indexOf('?');
        if (i > 0) {
          func = func.substring(0, i);
        }
        if(func =="toggle_led")
        {
          toggleLed(request);
        }
      }
      request->send(404, "text/plain", "Not found " + request->url());
    });
  
  server.begin();
}

void loop() {
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
      #ifdef WL_WRONG_PASSWORD
      case WL_WRONG_PASSWORD:
        DebugPrintln("Wrong WIFI password");
        break;
      #endif
      default:
        sprintf(buf, "%d,", wifiStatus);
        DebugPrint(buf);
        if((count++ % 80) == 0) DebugPrintln("");
    }
  }

  DebugPrintln("");
  #ifdef ESP8266
    sprintf(buf, "Connected to %s as %s: %s", WiFi.SSID().c_str(), WiFi.hostname().c_str(), WiFi.localIP().toString().c_str());
  #else //esp32
    sprintf(buf, "Connected to %s as %s: %s", WiFi.SSID().c_str(), WiFi.getHostname, WiFi.localIP().toString().c_str());
  #endif
  DebugPrintln(buf);
}

// set built-in LED light
void setLEDLight(bool isOn){
  digitalWrite(LED_BUILTIN, isOn?LOW:HIGH);
  isLEDOn = isOn;
}

// a web API to toggle built-in LED
void toggleLed(AsyncWebServerRequest *request) {
  DebugPrintln("Toggle LED");
  setLEDLight(!isLEDOn);
  request->send(200, "text/plain", isLEDOn?"true":"false");
}
