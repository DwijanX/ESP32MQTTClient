#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "POCO X4 Pro 5G";
const char* pass = "testwemos";

const char* MQTT_BROKER = "broker.hivemq.com";
const int MQTT_PORT = 1883;

const char* MQTT_CLIENT_ID = "grupo2_Leds";

const char* SUBSCRIBE_TOPIC_REDLED = "iot/grupo2/redLed";    // publish topic
const char* SUBSCRIBE_TOPIC_BLUELED = "iot/grupo2/blueLed";    // publish topic
const char* SUBSCRIBE_TOPIC_YELLOWLED = "iot/grupo2/yellowLed";    // publish topic

const int REDPIN=26;
const int BLUEPIN=17;
const int YELLOWPIN=25;

int currentLedOnPin;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

void turnLed(bool state,int ledPin)
{
      digitalWrite(ledPin, state);
}
void manageLedMessages(String instruction,int ledPin)
{
    if (instruction == "LED_ON") 
    {
      turnLed(1, ledPin);
      turnLed(0, currentLedOnPin);
      currentLedOnPin=ledPin;
    }
}

// PubSubClient callback function
void callback(const char* topic, byte* payload, unsigned int length) {
  String message;
  for (int i = 0; i < length; i++) 
  {
    message += String((char) payload[i]);
  }
  Serial.println("Message from topic " + String(topic) + " : " + message);
  if (String(topic) == SUBSCRIBE_TOPIC_REDLED) 
    manageLedMessages(message,REDPIN);
  else if (String(topic) == SUBSCRIBE_TOPIC_BLUELED)
    manageLedMessages(message,BLUEPIN);
  else if(String(topic) == SUBSCRIBE_TOPIC_YELLOWLED)
    manageLedMessages(message,YELLOWPIN);
}

boolean mqttClientConnect() {
  Serial.println("Connecting to " + String(MQTT_BROKER));
  if (mqttClient.connect(MQTT_CLIENT_ID)) 
  {
    Serial.println("Connected to " + String(MQTT_BROKER));

    mqttClient.subscribe(SUBSCRIBE_TOPIC_REDLED);
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPIC_REDLED));
    mqttClient.subscribe(SUBSCRIBE_TOPIC_BLUELED);
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPIC_BLUELED));
    mqttClient.subscribe(SUBSCRIBE_TOPIC_YELLOWLED);
    Serial.println("Subscribed to " + String(SUBSCRIBE_TOPIC_YELLOWLED));
  } 
  else 
  {
    Serial.println("Can't connect to " + String(MQTT_BROKER));
  }
  return mqttClient.connected();
}

void setup() {
  pinMode(REDPIN, OUTPUT);
  pinMode(YELLOWPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);
  Serial.begin(115200);

  Serial.println("Connecting to " + String(ssid));
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");

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

    if (now - previousPublishMillis >= 10000) {
      previousPublishMillis = now;
    }
  }
}
