#include <SparkFun_Bio_Sensor_Hub_Library.h>
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"

// No other Address options.
#define DEF_ADDR 0x55


// ***** Config sparkfun heart rate pulse oximeter sensor

// Reset pin, MFIO pin
const int resPin = 35;
const int mfioPin = 33;

// Takes address, reset pin, and MFIO pin.
SparkFun_Bio_Sensor_Hub bioHub(resPin, mfioPin); 

bioData body;  

// *****

// ***** Config DHT11 humidity temperature sensor

#define DHTPIN 13     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
//

// ***** Config OLED screen

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// *****

// ***** Config MQTT server

const char* ssid     = "Jirapong's Galaxy A33 5G";
const char* password = "xcds2664";

#define mqttServer    "192.168.173.49"
#define mqttPort      1883
#define mqttUser      ""
#define mqttPassword  ""
//#define device_id "tracker-001"
String device_id = "tracker-001";

WiFiClient wificlient;
PubSubClient mqttClient(wificlient);

// *****

// ***** config session id
int session_id;

void setup(){

  Serial.begin(115200);

  // ************ Setup OLED

  Wire.begin();

  Serial.println("Initializing OLED...");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();


  // ************ Setup pulseoximeter sensor
  
  int result = bioHub.begin();
  if (!result)
    Serial.println("Pulseoximeter sensor started!");
  else
    Serial.println("Could not communicate with the sensor!!!");

  Serial.println("Configuring Sensor...."); 
  int error = bioHub.configBpm(MODE_ONE); // Configuring just the BPM settings. 
  if(!error){
    Serial.println("Pulseoximeter sensor configured.");
  }
  else {
    Serial.println("Error configuring sensor.");
    Serial.print("Error: "); 
    Serial.println(error); 
  }
  // Data lags a bit behind the sensor, if you're finger is on the sensor when
  // it's being configured this delay will give some time for the data to catch
  // up. 

  // **************** Setup DHT11
  dht.begin();

  // **************** Setup Wifi

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

  // ************ Setup MQTT server
  Serial.println("Setting up MQTT server...");
  mqttClient.setServer(mqttServer, mqttPort);

  Serial.println("Initiate session...");

  // *********** Generate session_id everytime the device started
  session_id = random(0,10000000);
  Serial.print("Publishing session_id: ");
  Serial.println(session_id);
  // *********** Send latest session_id to MQTT broker
  mqtt_publish(("iot/register/"+device_id),String(session_id));

  delay(4000); 

}

void loop(){

    String text;
    float bt;
    int hr, spo2, confidence;

    // ****** Get temperature from DHT11
    bt = dht.readTemperature();
    bt = bt+2.5; // Correct skin temp -> core temp

    // ********* Get heart rate and SpO2 ************

    // Information from the readBpm function will be saved to our "body"
    // variable.  
    body = bioHub.readBpm();
    hr = body.heartRate;
    spo2 = body.oxygen;
    confidence = body.confidence;
    
    Serial.print("Heartrate: ");
    Serial.print(body.heartRate); 
    Serial.print(", Oxygen: ");
    Serial.print(body.oxygen); 
    Serial.print(", Confidence: ");
    Serial.println(body.confidence); 

    // *************** Output to OLED *************
    
    if (body.confidence >=90 && body.heartRate && body.oxygen) {
      text = "Heart rate: "+String(body.heartRate)+"\nOxygen: "+String(body.oxygen)+"\nTemp: "+String(bt);
      display_text(text);
      mqtt_publish_vital_signs(session_id,body.heartRate,body.oxygen,bt);
    }
    else display_text("Waiting for data");
    
    // *************** Send vital signs data to MQTT broker *************
    

    delay(5000); // Slowing it down, we don't need to break our necks here.
}

void display_text(String text) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(text);
  display.display();
}

void mqtt_publish_vital_signs(int session_id,int hr,int spo2, float bt) {
    String vital_signs;
    String topic;

    topic = ("iot/"+device_id+"/"+String(session_id));
    vital_signs = "{\"device_id\": \""+device_id+"\",\"session_id\": "+String(session_id)+",\"hr\": "+String(hr)+",\"spo2\": "+String(spo2)+",\"bt\":"+String(bt,2)+"}";

    mqtt_publish(topic,vital_signs);
}

void mqtt_publish(String topic,String payload) {
    while (!mqttClient.connected()) {
      Serial.println("Connecting to MQTT server...");
      mqttClient.connect(device_id.c_str(),mqttUser,mqttPassword);
    }
    if (mqttClient.publish(topic.c_str(),payload.c_str())) 
      Serial.println("MQTT published.");
}
