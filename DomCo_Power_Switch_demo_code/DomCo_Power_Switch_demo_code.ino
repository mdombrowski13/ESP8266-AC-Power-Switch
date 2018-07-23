/******************************************************
   Program: DomCo_Power_Switch_Default_Code
   C++_file: DomCo_Power_Switch_Default_Code.ino
   Version: 1.2.0
   Created: October 30th, 2016
   Created By: DomCo Electronics, Inc
               Techsupport@domcoelectronics.com
               
   Description:
   This program is designed to demo the capabilites
   of the DomCo Electronics Power Switch.  This code
   is provided free of charge.  DomCo Electornics, Inc.
   provides no support or liability for damages if the 
   code is used.

   WARNING!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   The DomCo Power Switch contains HIGH voltages.  NEVER
   touch any part of the circuitry or components while
   the Mains (AC power) is applied or ON.   Before 
   Programming, Servicing, or handling the board disconnect
   AC Mains from the unit.  AC power may still be present
   even after the Mains have been removed so caution and
   proper Mains handling is necessary.

   Copyright (c) 2016, DomCo Electronics, Inc.
 *****************************************************/

#include <ESP8266WiFi.h>          // https://github.com/esp8266/Arduino
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         // https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>

#define VERSION_MAJOR   1
#define VERSION_MINOR   2
#define VERSION_BUILD   0


//#define STATIC_IP_NETWORK           // Uncomment this out if you want to set a static IP address once it joins your network.
//#define CLEAR_SSID_ON_EVERY_BOOT    // Uncomment this code if you want the DomCo Power Switch to clear the stored SSID/PASSWORD every time the code is flashed.  This will cause it to boot to AP mode every time.

#define RELAY_PIN   13          // Relay Output Control Pin
#define RESET_PIN   12          // GND RESET_PIN to during power up to erase stored SSID/PASSWORD
#define LED_4       5           // AMBER LED
#define LED_5       4           // Green LED

#define RELAY_ON    HIGH        // Active high output for relay ON
#define RELAY_OFF   !RELAY_ON

#define LED_ON      LOW         // Active Low output for LED's
#define LED_OFF     !LED_ON

#ifdef STATIC_IP_NETWORK
  IPAddress _ip = IPAddress(192, 168, 1, 236);    // set static IP Address here
  IPAddress _gw = IPAddress(192, 168, 1, 1);      // set static IP Gateway here
  IPAddress _sn = IPAddress(255, 255, 255, 0);    // set static IP Subnet Mask here
#endif

ESP8266WebServer server(80);

String site;

String getMacAddress() 
{
  byte mac[6];

  WiFi.macAddress(mac);
  String cMac = "";
  for (int i = 0; i < 6; ++i) 
  {
    if (mac[i]<0x10) 
    {
      cMac += "0";
    }
  
  cMac += String(mac[i],HEX);
  if(i<5)
    cMac += ":"; // delimiter between bytes
  }
  
  cMac.toUpperCase();
  return cMac;
}

void create_WebSite()
{
  site = "<style>";
  site += ".submit-button {width: 90%; height: 150px; font-size: 50px;}";
  site += ".footer-link {bottom: 10; position: absolute; margin: auto; width: 99%;}";
  site += "</style>";
  site += "<body style=\"background-color:blue; font-size:50px\">";
  site += "<center>";
  site += "<h1 style=\"color:Red; padding-bottom:20px\">DomCo Relay Demo</h1>";
  site += "<h2 style=\"color:white; padding-bottom:40px\">IP Address: ";
  site += WiFi.localIP().toString();
  site += "</h2>";
  site += "<h2 style=\"color:white; padding-bottom:40px\">The Relay is ";
  if(digitalRead(RELAY_PIN) == RELAY_ON)
  {
    site += "On";
  }
  else
  {
    site += "Off";
  }
  site += "</h2>";
  site += "<form action=\"/On\" method=\"POST\"><input type=\"submit\" value=\"Turn Relay On\" class=\"submit-button\"></form>";
  site += "<form action=\"/Off\" method=\"POST\"><input type=\"submit\" value=\"Turn Relay Off\" class=\"submit-button\"></form>";
  site += "<h2 style=\"color:white; padding-bottom:40px\">MAC Address: ";
  site += getMacAddress();
  //site += WiFi.macAddress().toString();
  site += "</h2>";
  site += "<h2 style=\"color:white; padding-bottom:40px\">Version: ";
  site += VERSION_MAJOR;
  site += ".";
  site += VERSION_MINOR;
  site += ".";
  site += VERSION_BUILD;
  site += "</h2>";
  site += "<div class=\"footer-link\"><a href=\"http://www.domcoelectronics.com\" style=\"color:coral; font-weight:bold;\">DomCoElectronics.com</a></div>";   
  site += "</center></body>";
}

void handleRoot() 
{
  Serial.println("Default WebPage loaded");
  digitalWrite(LED_4, LED_ON);
  create_WebSite();
  server.send(200, "text/html", site);
  delay(100);
  digitalWrite(LED_4, LED_OFF);
}

void handleRELAYon() 
{ 
 Serial.println("Relay on WebPage");
 digitalWrite(RELAY_PIN, RELAY_ON);
 create_WebSite();
 server.send(200, "text/html", site);
 delay(100);
}
 
void handleRELAYoff() 
{ 
 Serial.println("Relay off WebPage");
 digitalWrite(RELAY_PIN, RELAY_OFF); 
 create_WebSite(); 
 server.send(200, "text/html", site);
 delay(100);
}

void handleNotFound()
{
  digitalWrite(LED_4, LED_ON);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(LED_4, LED_OFF);
}

void setup() 
{
    pinMode(RELAY_PIN, OUTPUT);         //output pin for the relay
    digitalWrite(RELAY_PIN, RELAY_OFF); //ste initial Relay state as off
    pinMode(RESET_PIN, INPUT);          //input pin to reset wifi credientals
    pinMode(LED_4, OUTPUT);
    digitalWrite(LED_4, LED_ON);
    pinMode(LED_5, OUTPUT);
    digitalWrite(LED_5, LED_OFF);

    Serial.begin(115200);               //set serial port default pin high
    
    WiFiManager wifiManager;            //Local intialization. Once its business is done, there is no need to keep it around

    if(digitalRead(RESET_PIN) == LOW)
    {
      wifiManager.resetSettings();      //reset saved settings
    }

#ifdef CLEAR_SSID_ON_EVERY_BOOT
    wifiManager.resetSettings();      //reset saved settings
#endif

#ifdef STATIC_IP_NETWORK
    wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
#endif

    wifiManager.autoConnect("DomCo_Relay");

    digitalWrite(LED_4, LED_OFF);

    Serial.println();
    if (MDNS.begin("DomCo_Relay")) 
    {
      Serial.println("MDNS responder started");
    }
    
    digitalWrite(LED_5, LED_ON);
    Serial.println("connected to Access Point:");
    Serial.println(WiFi.SSID());
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP

    server.on("/", handleRoot);
    server.on("/On", handleRELAYon); //as Per  <a href="ledOn">, Subroutine to be called
    server.on("/Off", handleRELAYoff);
    server.onNotFound(handleNotFound);
    
    server.begin();
    Serial.println("HTTP server started");
}

void loop() 
{
  server.handleClient();    
}
