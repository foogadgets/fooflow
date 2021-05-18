#include <Arduino.h>
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <PID_v1.h>
#include <WebSocketsServer.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "credentials.h"
#include "reflowProfiles.h"

#define SSR 5
#define DQ 0
#define SIZEOFTEMP 100

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

static const char *hostName = "fooflow";
char tmpBuf[SIZEOFTEMP];
static uint8_t stateToken = 0;
double Setpoint, Input, Output;
double Kp = 300, Ki = 0.1, Kd = 5;
PID myPID(&Input, &Output, &Setpoint, Kp, Ki, Kd, DIRECT);
OneWire oneWire(DQ);
DallasTemperature thermocouple(&oneWire);
uint32_t reflowStartTime = 0;
uint16_t pidWindowTime = 2000; // PWM frequency 0.5Hz
uint32_t pidWindowStartTime = 0;
uint8_t selectedProfile = 0;
uint8_t reflowProfileIndex = 0;
uint32_t tempSendTime = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;

  case WStype_CONNECTED:
  {
    IPAddress ip = webSocket.remoteIP(num);
    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
    // send message to client
    webSocket.sendTXT(num, "Connected");
  }
  break;

  case WStype_TEXT:
    switch (payload[0])
    {
    case '1':
      Serial.println("Got preheat trigger");
      reflowStartTime = millis();
      pidWindowStartTime = reflowStartTime;
      reflowProfileIndex = 0;
      stateToken = 2;
      break;

    case '2':
      Serial.println("Got start reflow trigger");
      reflowStartTime = millis();
      pidWindowStartTime = reflowStartTime;
      reflowProfileIndex = 0;
      stateToken = 1;
      break;

    case '3':
      Serial.println("Got stop trigger");
      reflowStartTime = 0;
      stateToken = 0;
      break;

    default:
      break;
    }
    break;

  default:
    break;
  }
}

void controlSSR() {
    if (millis() - pidWindowStartTime >= pidWindowTime)
    { //time to shift the Relay Window
      pidWindowStartTime += pidWindowTime;
    }
    if (Output < millis() - pidWindowStartTime)
      digitalWrite(SSR, LOW);
    else
      digitalWrite(SSR, HIGH);
}

void sendValues() {
    memset(tmpBuf, 0, SIZEOFTEMP * sizeof(char));
    strncpy(tmpBuf, "t:", 2);
    sprintf(&tmpBuf[2], "%0.1f:%0.0f:%0.0f:%0.2f:%0.0f", Input, Setpoint, Kp, Ki, Kd);
    webSocket.sendTXT(0, tmpBuf);
}


void setup()
{
  Serial.begin(115200);

  FS *fileSystem = &SPIFFS;
  SPIFFSConfig fileSystemConfig = SPIFFSConfig();
  fileSystemConfig.setAutoFormat(false);
  fileSystem->setConfig(fileSystemConfig);

  fileSystem->begin();

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println();
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // start webSocket server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  if (MDNS.begin(hostName))
  {
    Serial.print("Browse to:  ");
    Serial.print("http://");
    Serial.print(hostName);
    Serial.println(".local");
  }

  server.serveStatic("/", SPIFFS, "/web/");
  server.serveStatic("/images/", SPIFFS, "/images/");
  server.serveStatic("/css/", SPIFFS, "/css/");
  server.serveStatic("/scripts/", SPIFFS, "/scripts/");

  server.begin();

  MDNS.addService("http", "tcp", 80);
  MDNS.addService("ws", "tcp", 81);

  thermocouple.begin();
  thermocouple.setResolution(12);

  pinMode(SSR, OUTPUT);
  digitalWrite(SSR, 0);

  myPID.SetOutputLimits(0, pidWindowTime);
  myPID.SetMode(AUTOMATIC);

  tempSendTime = millis();
}

void loop()
{
  switch (stateToken)
  {
  case 1: // Reflow
    thermocouple.requestTemperatures();
    if ((millis() - reflowStartTime) / 1000 > myReflowProfile[selectedProfile].timeProfile[reflowProfileIndex])
    {
      if (5 == reflowProfileIndex++)
      {
        stateToken = 0;
        tempSendTime = millis();
        break;
      }
    }
    Setpoint = myReflowProfile[selectedProfile].tempProfile[reflowProfileIndex];
    Input = thermocouple.getTempCByIndex(0);
    myPID.Compute();
    controlSSR();
    sendValues();
    break;

  case 2: // Pre-heat
    thermocouple.requestTemperatures();
    Input = thermocouple.getTempCByIndex(0);
    Setpoint = 80;
    myPID.Compute();
    controlSSR();
    sendValues();
    if (4 *60*1000 < (millis()-reflowStartTime)) stateToken = 0;
    break;

  default:
    digitalWrite(SSR, 0);
    reflowProfileIndex = 0;
    if (millis()-tempSendTime >= 2000) {
      thermocouple.requestTemperatures();
      Setpoint = 0;
      Input = thermocouple.getTempCByIndex(0);
      sprintf(&tmpBuf[2], "%0.1f:%0.0f", Input, Setpoint);
      webSocket.sendTXT(0, tmpBuf);
      tempSendTime += 2000;
    }
    break;
  }

  webSocket.loop();
  server.handleClient();
  MDNS.update();
  myPID.Compute();
}
