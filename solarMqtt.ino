#include "secret.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "WiFi.h"
 

 
#define AWS_IOT_PUBLISH_TOPIC   "esp32/pub"
#define AWS_IOT_SUBSCRIBE_TOPIC "esp32/sub"
 

int minutes = 5;
int minute = 60000;

WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);
 
void connectAWS()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  Serial.println("Connecting to Wi-Fi");
 
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
 
  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);
 
  // Connect to the MQTT broker on the AWS endpoint we defined earlier
  client.setServer(AWS_IOT_ENDPOINT, 8883);
 
  // Create a message handler
  client.setCallback(messageHandler);
 
  Serial.println("Connecting to AWS IOT");
 
  while (!client.connect(THINGNAME))
  {
    Serial.print(".");
    delay(100);
  }
 
  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    return;
  }
 
  // Subscribe to a topic
  client.subscribe(AWS_IOT_SUBSCRIBE_TOPIC);
 
  Serial.println("AWS IoT Connected!");
}
 
void publishMessage()
{
  StaticJsonDocument<200> doc;
  
  doc["deviceId"] = 1234567;
  doc["battery"]["voltage"] = 11.2;
  doc["battery"]["charge"] = 80.0;
  doc["speed"]["download"] = 6.0;
  doc["speed"]["upload"] = 6.2;
  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer); // print to client
  if(WiFi.status() != WL_CONNECTED){
    Serial.println("Wifi disconnected");
  }else{
    Serial.println("Wifi connected");
  }
  if(!client.connect(THINGNAME)){
    Serial.println("Client disconected");
  }else{
    Serial.println("Client connected");
  }
  bool publish_status = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  while (!publish_status){
    Serial.println("Need to connect again");
    connectAWS();
    publish_status = client.publish(AWS_IOT_PUBLISH_TOPIC, jsonBuffer);
  }
}
 
void messageHandler(char* topic, byte* payload, unsigned int length)
{
  Serial.print("incoming: ");
  Serial.println(topic);
 
  StaticJsonDocument<200> doc;
  deserializeJson(doc, payload);
  const char* message = doc["message"];
  Serial.println(message);
}

int count = 0;
void setup()
{
  Serial.begin(115200);
  connectAWS();
  
}
 
void loop()
{ 
 
  Serial.println("Count: " + String(count));
  publishMessage();
  client.loop();
  for (int i=0;i<minutes;i++){
    delay(minute);
  }
  count = count + 1;
}
