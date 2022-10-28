#include <PubSubClient.h>

#define MQTT_SERVER ""
#define MQTT_PORT 1883

#define TOPIC "/save"

String POSTDATA = "sendNewDeviceData";
String POSTIMAGE = "sendNewImg";
String UPDATE = "updateDeviceData";

String messBuff = "";


void connect_to_broker(PubSubClient &client) {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "save";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe(MQTT_LED1_TOPIC);
      client.subscribe(MQTT_LED2_TOPIC);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      delay(2000);
    }
  }
}

void callback(char* topic, byte *payload, unsigned int length) {
  char status[20];
  Serial.println("-------new message from broker-----");
  Serial.print("topic: ");
  Serial.println(topic);
  Serial.print("message: ");
  Serial.write(payload, length);
  Serial.println();
  messBuff = "";
  for(int i = 0; i<length; i++)
  {
    messBuff += (char)payload[i];
  }
  if (messBuff == POSTDATA) return 1;
  else if (messBuff ==  POSTIMAGE) return 2;
  else if (messBuff == UPDATE) return 3;
}
