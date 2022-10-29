// Link fastled: https://github.com/FastLED/FastLED/archive/master.zip

#include "Light.h"
#include "DHT.h"
#include "Waveshare_SIM7600.h"
#include <Arduino_JSON.h>
#include "fastled.h"
#include "esp_timer.h"
#include "readPIN.h"
#include <PubSubClient.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "RTC.h"
#include "HTTP_FIREBASE.h"
#include "cam.h"

#define MQTT_SERVER "test.mosquitto.org"
#define MQTT_PORT 1883

const char* ssid = "KTMT - SinhVien";
const char* password = "sinhvien";




#define TOPIC "/save"

#define RAINPIN 14
#define RELAYPIN 4
#define DHTTYPE DHT22
#define DHTPIN 2
#define myID "save"


String POSTDATA = "sendNewDeviceData";
String POSTIMAGE = "sendNewImg";
String UPDATE = "updateDeviceData";
#define WAKE_UP 4

String ID = "save";

#define uS_TO_S_FACTOR 1000000ULL


//WiFiClient wifiClient;
//PubSubClient MQTTClient(wifiClient);



// GPS variable
//String strLat, strLog, strTime, Date;
String hexled = "0xFF00FF";
int brightness = 100;
DHT dht(DHTPIN, DHTTYPE);
JSONVar PACKET;
JSONVar GPS;
JSONVar Data;
uint8_t WakeUpTime;
uint8_t SleepTime;
String MQTTMessage = "";
bool isRaining = false;
bool GridStatus = false;
int current_time = 650;

//timer set up
esp_timer_create_args_t periodic_timer_args;
esp_timer_handle_t periodic_timer;
int timer_value = 20;
int wakeup_time = 0;
int sleep_time = 620;
bool is_timer_start = false;

esp_sleep_wakeup_cause_t wakeup_cause;
bool flagsleep = false;
int MessStatus = 0;

String MQTTMess = "";

String URL = "https://inset-catch-electric.herokuapp.com/updateDeviceData";

void SendDataFB(WiFiClient wifi,String buff) {
  HTTPClient http;
  http.begin(URL);
  http.addHeader("Content-Type","application/json");
  int responeCode = http.POST(buff);
  Serial.println(responeCode);
}

void Get_SendDataSensor()
{
  PACKET["id"] = ID;
  float h = dht.readHumidity();
  Data["humi"] = h;

  float t = dht.readTemperature();
  Data["temp"] = t;

  //get image
 // Data["img"] = Photo2Base64();

  float light = 15;
  Data["optic"] = light;

  //  GPS read data
//  sim7600.GPSPositioning(Lat, Log, Date, Time);
//  if (Lat != 0 && Log != 0) {
//    strLat = String(Lat, 6);
//    GPS["latitude"] = "10.21414";
//    strLog = String(Log, 6);
//    GPS["longitude"] = "213.2313131";
//    strTime = String(Time, 0);
//    String hourTime = strTime.substring(0, 1);
//    current_time = hourTime.toInt() * 60;
//    String minTime = strTime.substring(2, 3);
//    current_time += minTime.toInt();
//    //GPS["Time"] = strTime;
//    //GPS["Date"] = Date;
//  }

//  Data["coordinates"] = GPS;
  Data["battery"] = 83;
  Data["rain"] = isRaining;
  Data["statusGrid"] = GridStatus;
  PACKET["data"] = Data;
//  SendDataFB(wifiClient,"{\"id\":\"save\",\"data\":{\"desc\":\"khanhbangdan\",\"battery\":83,\"humi\":34,\"temp\":34,\"optic\":163,\"rain\":false,\"statusGrid\":true}}");

}

void setup_wifi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

//void connect_to_broker() {
//  while (!MQTTClient.connected()) {
//    Serial.print("Attempting MQTT connection...");
//    String clientId = "save";
//    clientId += String(random(0xffff), HEX);
//    if (MQTTClient.connect(clientId.c_str())) {
//      Serial.println("connected");
//      MQTTClient.subscribe(TOPIC);
//    } else {
//      Serial.print("failed, rc=");
//      Serial.print(MQTTClient.state());
//      Serial.println(" try again in 2 seconds");
//      delay(2000);
//    }
//  }
//}

void callback(char* topic, byte *payload, unsigned int length) {
  char status[20];
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  MQTTMess = "";
  for(int i = 0; i<length; i++)
  {
    MQTTMess += (char)payload[i];
  }
  if (MQTTMess.indexOf(POSTDATA)) MessStatus = 1;
  else if (MQTTMess.indexOf(POSTIMAGE)) MessStatus = 2;
  else if (MQTTMess.indexOf(UPDATE)) MessStatus = 3;
}

//void setup_MQTT() {
////  MQTTClient.setServer(MQTT_SERVER, MQTT_PORT);
//  MQTTClient.setCallback(callback);
//  connect_to_broker();
//}

void SendImage2FB(String mess) {
  JSONVar Image;
  Image["img"] = Photo2Base64();
  JSONVar Data;
  Data["id"] = ID;
  Data["data"] = Image;
}

void update_data(String mess) {
  int idx = mess.indexOf('\n');
  int idx1 = mess.indexOf("0x");
  String ledColor = mess.substring(idx1, idx1 + 8);
  Serial.println(ledColor);
  hexled = ledColor;

  idx1 = mess.indexOf("brightness");
  String temp = mess.substring(idx1);
  int t_idx = temp.indexOf(':');
  int t_idx2 = temp.indexOf(',');
  String brightness_str = temp.substring(t_idx + 1, t_idx2);
  Serial.println(brightness_str);
  brightness = brightness_str.toInt() * 100 / 255;
  Config_led(hexled, brightness);

  idx1 = mess.indexOf("timeSend");
  temp = mess.substring(idx1);
  t_idx = temp.indexOf(':');
  t_idx2 = temp.indexOf(',');
  String timeSend = temp.substring(t_idx + 1, t_idx2);
  Serial.println(timeSend);
  timer_value = timeSend.toInt() * 60;
  if (is_timer_start) {
    ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
    is_timer_start = false;
  }

  idx1 = mess.indexOf("hour");
  temp = mess.substring(idx1);
  t_idx = temp.indexOf(':');
  t_idx2 = temp.indexOf(',');
  String hourStart = temp.substring(t_idx + 1, t_idx2);
  Serial.println(hourStart);
  idx1 = mess.indexOf("min");
  temp = mess.substring(idx1);
  t_idx = temp.indexOf(':');
  t_idx2 = temp.indexOf('}');
  String minStart = temp.substring(t_idx + 1, t_idx2);
  Serial.println(minStart);
  wakeup_time = hourStart.toInt() * 60 + minStart.toInt();

  int idx_n = mess.indexOf("timeEnd");
  String temp2 = mess.substring(idx_n);
  Serial.println(temp2);
  idx1 = temp2.indexOf("hour");
  temp2 = temp2.substring(idx1);
  t_idx = temp2.indexOf(':');
  t_idx2 = temp2.indexOf(',');
  String hourEnd = temp2.substring(t_idx + 1, t_idx2);
  Serial.println(hourEnd);
  idx1 = temp2.indexOf("min");
  temp2 = temp2.substring(idx1);
  t_idx = temp2.indexOf(':');
  t_idx2 = temp2.indexOf('}');
  String minEnd = temp2.substring(t_idx + 1, t_idx2);
  Serial.println(minEnd);
  sleep_time = hourEnd.toInt() * 60 + minEnd.toInt();
}



String Data2JSON(String key, String value) {
  return "{\"" + key + "\":\"" + value + "\"}";
}


void periodic_timer_callback(void* arg)
{
    int64_t time_since_boot = esp_timer_get_time();
    printf("Periodic timer called, time since boot: %lld us\n", time_since_boot);
    Get_SendDataSensor();
}

esp_sleep_wakeup_cause_t sleep() {
  while (Serial.available()) Serial.read();

  esp_sleep_enable_timer_wakeup(timer_value * uS_TO_S_FACTOR);
  delay(100);
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_GPIO3);
  gpio_wakeup_enable(GPIO_NUM_3, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  // Enter light sleep mode
  esp_err_t check_sleep = esp_light_sleep_start();
  // Restore GPIO3 function as U0RXD
  PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD_U0RXD);
  esp_sleep_wakeup_cause_t wakeup_cause;
  wakeup_cause = esp_sleep_get_wakeup_cause();
  //Serial.println("Wakeup");
  return wakeup_cause;
}

void setup() {
  Serial.begin(115200);
  dht.begin();
//  setup_wifi();
  camera_init();
  delay(15000);
//  setup_MQTT();

  setup_fastled();
  Config_led(hexled, brightness);
  setup_rtc();
  //sim7600.PowerOn();
  delay(500);
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, HIGH);
  periodic_timer_args.callback = &periodic_timer_callback;
            /* name is optional, but may help identify the timer when debugging */
  periodic_timer_args.name = "periodic";
  ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
  PIN_setup();
}


void loop() {
  if  (flagsleep) {
    if (is_timer_start) {
      ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
      is_timer_start = false;
    }
    Serial.println("Sleep");
      wakeup_cause = sleep();
      
      if (wakeup_cause == ESP_SLEEP_WAKEUP_TIMER) {
        Serial.println("Timer wake up");
        Get_SendDataSensor();
        // get time
        // if (gps_time > wakeup_time) flagsleep = false;
        if (GetLocalTime() > wakeup_time) flagsleep = false;
      }
      if (wakeup_cause == ESP_SLEEP_WAKEUP_GPIO) {
       //TEST 
//        int mess = recieved(MQTTMessage);
        delay(1000);
          
          Get_SendDataSensor();
//        else if (mess == IMAGE) post_data(Photo2Base64());
//        else if (mess == UPDATE) update_data(MQTTMessage);
      }
    }
  if (!flagsleep) {
    if (!is_timer_start) {
      ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, timer_value * uS_TO_S_FACTOR));
      is_timer_start = true;
    }
//    if (MessStatus == 1) Get_SendDataSensor();
    else if (MessStatus == 2);
    else if (MessStatus == 3) update_data(MQTTMess);
    int rain = digitalRead(RAINPIN);
    if (rain == LOW && GridStatus == true) {
      // dang mua
      // tat luoi
      Serial.println("Dang mua");
      digitalWrite(RELAYPIN, LOW);
      isRaining = true;
      GridStatus = false;
    }
    else if (rain == HIGH && GridStatus == false) {
      digitalWrite(RELAYPIN, HIGH);
      Serial.println("Troi khong mua");
      isRaining = false;
      GridStatus = true;
    }
  }
}
