#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>

#define STASSID "ConfigWiFi"
#define STAPSK "codetyphon"
#define MDNSNAME "MopsPower"

const char *ssid = STASSID;
const char *password = STAPSK;
const char *name = MDNSNAME;
const int port = 18650;
ESP8266WebServer server(port);

const int led = 12;
const int relay = 15;
const int btn = 13;

int buttonState = LOW;
int powerState = LOW;

void handleRoot()
{
  server.send(200, "text/plain", "mops switch!");
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void turnon()
{
  Serial.println("turn on");
  digitalWrite(led, LOW);    //led是低电平触发
  digitalWrite(relay, HIGH); //继电器是高电平触发
  powerState = HIGH;
}

void turnoff()
{
  Serial.println("turn off");
  digitalWrite(led, HIGH);
  digitalWrite(relay, LOW);
  powerState = LOW;
}

void setup(void)
{
  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(btn, INPUT);

  turnoff();

  Serial.begin(9600);

  WiFiManager wifiManager;
  wifiManager.autoConnect(ssid, password);
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(name))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/on", []() {
    turnon();
    server.send(200, "text/plain", "power on");
  });

  server.on("/off", []() {
    turnoff();
    server.send(200, "text/plain", "power off");
  });

  server.on("/status", []() {
    server.send(200, "text/plain", String(powerState));
  });

  server.onNotFound(handleNotFound);
  server.begin();
  MDNS.addService("http", "tcp", port);
  Serial.println("HTTP server started");
}

void loop(void)
{
  MDNS.update();
  server.handleClient();
  buttonState = digitalRead(btn);
  if (buttonState == LOW)
  {
    Serial.println("button click");
    if (powerState == LOW)
    {
      turnon();
    }
    else
    {
      turnoff();
    }
    delay(1000);
  }
}
