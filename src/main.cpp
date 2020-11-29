#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#define STASSID "yourwifis"
#define STAPSK "yourpassword"

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

const int led = 12;
const int relay = 15;
const int btn = 13;

int buttonState = 0;
int powerState = 0;

void handleRoot()
{
  server.send(200, "text/plain", "hello from esp8266!");
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
  digitalWrite(led, 1);
  digitalWrite(relay, 1);
  powerState = 1;
}

void turnoff()
{
  digitalWrite(led, 0);
  digitalWrite(relay, 0);
  powerState = 0;
}

void setup(void)
{
  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(btn, INPUT);
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("connection to wifi...");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(led, 1);
    delay(500);
    digitalWrite(led, 0);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  turnoff();

  if (MDNS.begin("esp8266"))
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
  Serial.println("HTTP server started");
}

void loop(void)
{
  server.handleClient();
  MDNS.update();
  buttonState = digitalRead(btn);
  if (buttonState == HIGH)
  {
    if (powerState == 0)
    {
      turnon();
    }
    else
    {
      turnoff();
    }
  }
}