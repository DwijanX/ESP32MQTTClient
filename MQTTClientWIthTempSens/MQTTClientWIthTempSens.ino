#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#define DHTTYPE DHT11
#define TEMPERATUREPIN 5
#define MIDDLETEMPERATURELOWLIMIT 15 
#define MIDDLETEMPERATURETOPLIMIT 25 
const char* WIFI_SSID = "POCO X4 Pro 5G";
const char* WIFI_PASS = "testwemos";

const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

const char* MQTT_CLIENT_ID = "grupo2_tempsens";
const char* SUBSCRIBE_TOPIC_TEMP = "iot/grupo2/temp/in";    // publish topic

const char* PUBLISH_TOPIC_TEMP = "iot/grupo2/temp/out";    // publish topic
const char* PUBLISH_TOPIC_REDLED = "iot/grupo2/redLed";    // publish topic
const char* PUBLISH_TOPIC_BLUELED = "iot/grupo2/blueLed";    // publish topic
const char* PUBLISH_TOPIC_YELLOWLED = "iot/grupo2/yellowLed";    // publish topic



DHT dht(TEMPERATUREPIN, DHTTYPE);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);


void handlePostsBasedOn(float temperature)
{
  String tempString = String(temperature);
  mqttClient.publish(PUBLISH_TOPIC_TEMP, tempString.c_str());
  String ledOnString=String("LED_ON");
  if(temperature>MIDDLETEMPERATURETOPLIMIT)
    mqttClient.publish(PUBLISH_TOPIC_REDLED, ledOnString.c_str());
  else if(temperature<=MIDDLETEMPERATURETOPLIMIT && temperature>=MIDDLETEMPERATURELOWLIMIT)
    mqttClient.publish(PUBLISH_TOPIC_YELLOWLED, ledOnString.c_str());
  else if(temperature<MIDDLETEMPERATURELOWLIMIT)
    mqttClient.publish(PUBLISH_TOPIC_BLUELED, ledOnString.c_str());
}

// PubSubClient callback function
void callback(const char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) 
  {
    message += String((char) payload[i]);
  }
  Serial.println("Message from topic " + String(topic) + " : " + message);
  if (String(topic) == SUBSCRIBE_TOPIC_TEMP && message=="read")
  {
    float temperature = dht.readTemperature(); 
    while(isnan(temperature))
    {
      temperature = dht.readTemperature(); 
    }
    Serial.print("temperature: ");
    Serial.println(temperature);
    handlePostsBasedOn(temperature);
  }
}

boolean mqttClientConnect() {
  Serial.println("Connecting to " + String(MQTT_BROKER));
  if (mqttClient.connect(MQTT_CLIENT_ID)) 
  {
    Serial.println("Connected to " + String(MQTT_BROKER));

    mqttClient.subscribe(SUBSCRIBE_TOPIC_TEMP);
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPIC_TEMP));
  } 
  else 
  {
    Serial.println("Can't connect to " + String(MQTT_BROKER));
  }
  return mqttClient.connected();
}

void setup() {
  pinMode(TEMPERATUREPIN,INPUT); 
  Serial.begin(115200);

  Serial.println("Connecting to " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("Connected to " + String(WIFI_SSID));

  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
  mqttClient.setCallback(callback);
}

unsigned long previousConnectMillis = 0;
unsigned long previousPublishMillis = 0;

unsigned char counter = 0;

void loop() {
  unsigned long now = millis();
  if (!mqttClient.connected()) {
    if (now - previousConnectMillis >= 2000) {
      previousConnectMillis = now;
      if (mqttClientConnect()) previousConnectMillis = 0;
      else delay(1000);
    }
  } 
  else { // mqtt is connected
    mqttClient.loop();
    delay(20);
  }
}
