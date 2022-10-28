
#include "Base64.h"
#include <WiFi.h>
#include <HTTPClient.h>

String URL = "https://inset-catch-electric.herokuapp.com/updateDeviceData";
String stringRecv = "";
bool isRecieved = false;

bool USE_SSL = false;
#define DELAY_MS 4000

void SendDataFB(WiFiClient &wifi,String buff) {
  HTTPClient http;
  http.begin(wifi,URL);
  http.addHeader("Content-Type","application/json");
  int responeCode = http.POST(buff);
}
