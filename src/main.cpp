#include <Arduino.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

#define MDNSNAME "MopsPower"
#define AP_SSID "MopsPower"
#define AP_PASS "codetyphon"
const char *name = MDNSNAME;
const char *PARAM_POWER = "power";
const int port = 18650;
AsyncWebServer server(port);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

const int led = 12;
const int relay = 15;
const int btn = 13;

int buttonState = LOW;
int powerState = LOW;

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
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS);

  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(btn, INPUT);

  turnoff();

  Serial.begin(9600);
  Serial.println("");
  Serial.println(WiFi.localIP());

  if (MDNS.begin(name))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(200, "text/plain", "MopsPower"); });

  server.on("/api/status", HTTP_POST, [](AsyncWebServerRequest *request)
            {
              String power;
              String message;
              if (request->hasParam(PARAM_POWER))
              {
                power = request->getParam(PARAM_POWER)->value();
                if (power == "on")
                {
                  turnon();
                  message = "on";
                }
                if (power == "off")
                {
                  turnoff();
                  message = "off";
                }
              }
              else
              {
                message = "err";
              }
              request->send(200, "text/plain", message);
            });

  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              String state;
              state = String(powerState);
              request->send(200, "text/plain", "Hello, GET: " + state);
            });

  server.onNotFound(notFound);
  AsyncElegantOTA.begin(&server); // Start ElegantOTA
  server.begin();
  MDNS.addService("http", "tcp", port);
  Serial.println("HTTP server started");
}

void loop(void)
{
  MDNS.update();
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
