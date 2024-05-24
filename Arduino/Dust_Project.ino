#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "DHT.h"
#include <SoftwareSerial.h>
const int rxPin = D7;  // Replace with your receive pin

SoftwareSerial mySerial(rxPin); 
int status = 1;
int ddht = 14;   
String autopump = "on";

int measurePin = A0;
int ledPower = 16;

unsigned int samplingTime = 280;
unsigned int deltaTime = 40;
unsigned int sleepTime = 9680;

float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;

#define DHTTYPE DHT22

DHT dht(ddht, DHTTYPE);


const char* ssid = "CUHomeWiFi(2.4G)-1012";
const char* password = "430186791";

const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "0d8601a4-f49d-41ae-918b-ede8a9c1b682";
const char* mqtt_username = "rvRQAL8bXkogNHq3L6LTmUdyWvG6zyu9";
const char* mqtt_password = "fURpXpCB1G3f3NxREetwXogMBp7RrGx9";

WiFiClient espClient;
PubSubClient client(espClient);

char msg[100];

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection…");
    if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("@msg/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  String tpc;
  for (int i = 0; i < length; i++) {
    message = message + (char)payload[i];
  }
  Serial.println(message);
}

void setup() {
  mySerial.begin(115200); 
  pinMode(ledPower, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
   if (mySerial.available()){
      status = status + 1;
      status = status % 2 ;
      Serial.println(status);
      mySerial.read();
   }
   float h,t;
  String data;
  if(status == 1){
  h = dht.readHumidity();
  t = dht.readTemperature();
  
  digitalWrite(ledPower, LOW);
  delayMicroseconds(samplingTime);

  voMeasured = analogRead(measurePin);

  delayMicroseconds(deltaTime);
  digitalWrite(ledPower, HIGH);
  delayMicroseconds(sleepTime);

  calcVoltage = voMeasured * (5.0 / 1024);
  dustDensity = 170 * calcVoltage - 0.1;

  if (dustDensity < 0) {
    dustDensity = 0.00;
  }
}
  data = "{\"data\":{\"Humidity\": " + String(h) + ",\"Temperature\": " + String(t) + ",\"DustDensity\": " + String(dustDensity) + ",\"Status\": " + String(status) + "}}";
  
 
  Serial.println(data);
  data.toCharArray(msg, (data.length() + 1));
  client.publish("@shadow/data/update", msg);
  client.loop();
  delay(1000);
  
}
