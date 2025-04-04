#include "MqttClient.h"
#include "WifiClient.h"
#include <ArduinoJson.h>
#include <MQUnifiedsensor.h>

// Setting up MQ9 (taken from the Examples provided by MQ libraries)
#define         Board                   ("Arduino UNO")
#define         Pin                     (A0)  //Analog input 4 of your arduino
#define         Type                    ("MQ-9") //MQ9
#define         Voltage_Resolution      (5)
#define         ADC_Bit_Resolution      (10) // For arduino UNO/MEGA/NANO
#define         RatioMQ9CleanAir        (9.6) //RS / R0 = 60 ppm 
MQUnifiedsensor MQ9(Board, Voltage_Resolution, ADC_Bit_Resolution, Pin, Type);


char *ssid_wifi = "username";
char *pass_wifi = "password";

const int fanPin = D8;
const int airSensorPinMQ9 = D9;
const int airSensorPinMQ135 = D10;
const int analogPin = A0;


const char *mqtt_broker_ip = "192.168.2.214";
const int mqtt_broker_port = 1883;
const int num_subscribe_topics = 1;
String subscribe_topics[num_subscribe_topics] = {"turn_fan"};
WifiClient wifi_client(ssid_wifi, pass_wifi);
MqttClient mqtt_client(mqtt_broker_ip, mqtt_broker_port, subscribe_topics, num_subscribe_topics);
const char *client_id = "air_quality_sensor";
const char *topic = "tester_topic";
DynamicJsonDocument tester_json(1024);
DynamicJsonDocument msg_doc(1024);
unsigned long endFanTime = millis();

void setup() {
  pinMode(fanPin, OUTPUT);
  pinMode(airSensorPinMQ9, INPUT);
  pinMode(airSensorPinMQ135, INPUT);
  Serial.begin(9600);
  wifi_client.connect();
  mqtt_client.connect(client_id);

  // Calibrating the MQ9 Sensor and finding R0

  MQ9.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ9.init(); 
    Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ9.update(); // Update data, the arduino will read the voltage from the analog pin
    calcR0 += MQ9.calibrate(RatioMQ9CleanAir);
    Serial.print(".");
  }
  MQ9.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);}
  /*****************************  MQ CAlibration ********************************************/ 
  Serial.println("** Values from MQ-9 ****");
  Serial.println("|    LPG   |  CH4 |   CO  |"); 

}

void loop() {

  // Check MQTT connection
  mqtt_client.check_connection(client_id);
  // Check wifi connectionc
  if (!wifi_client.check_connection()) {
    Serial.println("WiFi disconnected. Reconnecting...");
    wifi_client.connect(); 
  }
  
  char serializedString [500];


  int gasState = digitalRead(airSensorPinMQ9);
  if (gasState == LOW) {
    Serial.println("MQ9: Gas Detected!");
    tester_json["GasDetected"] = true;
  } else {
    Serial.println("MQ9: Air is Clean");
    tester_json["GasDetected"] = false;
  }

  MQ9.update(); // Update data, the arduino will read the voltage from the analog pin
  /*
  Exponential regression:
  GAS     | a      | b
  LPG     | 1000.5 | -2.186
  CH4     | 4269.6 | -2.648
  CO      | 599.65 | -2.244
  */

  MQ9.setA(1000.5); MQ9.setB(-2.186); // Configure the equation to to calculate LPG concentration
  float LPG = MQ9.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ9.setA(4269.6); MQ9.setB(-2.648); // Configure the equation to to calculate LPG concentration
  float CH4 = MQ9.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  MQ9.setA(599.65); MQ9.setB(-2.244); // Configure the equation to to calculate LPG concentration
  float CO = MQ9.readSensor(); // Sensor will read PPM concentration using the model, a and b values set previously or from the setup

  tester_json["LPG"] = LPG;
  tester_json["CH4"] = CH4;
  tester_json["CO"] = CO;
  // Serial.println(tester_json);
  int airQualityState = digitalRead(airSensorPinMQ135);
  if (airQualityState == LOW) {
    Serial.println("MQ135: Poor air quality detected!");
    tester_json["AirQuality"] = false;
  } else {
    Serial.println("MQ135: Air quality is okay.");
    tester_json["AirQuality"] = true;
  }
  serializeJson(tester_json, serializedString);
  Serial.println(serializedString);
  mqtt_client.publish_message(topic, serializedString);
  delay (1000);


  // Deciding to turn the fan on:
  String msg = mqtt_client.get_msg();
  String topic = mqtt_client.get_topic();
  deserializeJson(msg_doc, msg);

  Serial.println(topic);
  if (topic == subscribe_topics[0]) {
    bool fanSwitch = msg_doc["fan"];
    if (fanSwitch == 1) {
      Serial.println("Fan is on");
      endFanTime = millis() + 5000;
      digitalWrite(fanPin, HIGH); // Turn fan ON
    }
    else{
      digitalWrite(fanPin, LOW);  // Keep fan OFF
    }
  }

  mqtt_client.reset_msg();

  if (endFanTime<millis()) {
    Serial.println("Fan is off.");
    digitalWrite(fanPin, LOW);  // Turn fan OFF
  }
  
  
}
