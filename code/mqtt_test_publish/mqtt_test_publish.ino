#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid     = "Jirapong's Galaxy A33 5G";
const char* password = "xcds2664";

#define mqttServer    "192.168.173.49"
#define mqttPort      1883
#define mqttUser      ""
#define mqttPassword  ""
#define device_id "tracker-001"

String msg;

WiFiClient wificlient;
PubSubClient mqttClient(wificlient);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

//  setupMQTT();
  mqttClient.setServer(mqttServer, mqttPort);
}

void loop() {

  String vital_signs;

//  vital_signs = "tracker-001,8,76,97,36.5";
  vital_signs = "{\"device_id\": \"tracker-001\",\"session_id\": 8,\"hr\": 76,\"spo2\": 96,\"bt\":36.5}";

  mqttClient.loop();
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT server...");
    mqttClient.connect(device_id,"","");
  }
  if (mqttClient.publish(("iot/"+String(device_id)+"/8").c_str(),vital_signs.c_str())) Serial.println("Publish successful");
  delay(10000);
}
