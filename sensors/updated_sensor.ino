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


char *ssid_wifi = <WIFI>;
char *pass_wifi = <PASSWORD>;

const int fanPin = D8;
const int airSensorPinMQ9 = D9;
const int airSensorPinMQ135 = D10;
const int analogPin = A0;


const char *mqtt_broker_ip = <RPI>;
const int mqtt_broker_port = 1883;

WifiClient wifi_client(ssid_wifi, pass_wifi);
MqttClient mqtt_client(mqtt_broker_ip, mqtt_broker_port);
const char *client_id = "air_quality_sensor";
const char *topic = "tester_topic";
DynamicJsonDocument air_quality_json(1024);

void setup() {
  // pinMode(fanPin, OUTPUT);
  // pinMode(airSensorPinMQ9, INPUT);
  pinMode(airSensorPinMQ135, INPUT);
  Serial.begin(9600);
  wifi_client.connect();
  mqtt_client.connect(client_id);

  // Calibrating the MQ9 Sensor and finding R0
  MQ9.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ9.init(); 
    Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++) {
    # Collect data for the first 10 points to find the callibration point
    MQ9.update();
    calcR0 += MQ9.calibrate(RatioMQ9CleanAir);
    Serial.print(".");
  }
  MQ9.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Connection Issue"); while(1);}
  if(calcR0 == 0){Serial.println("Conection issue found"); while(1);}

}

void loop() {

  mqtt_client.check_connection(client_id);
  char serializedString [500];
  
  int gasState = digitalRead(airSensorPinMQ9);
  if (gasState == LOW) {
    Serial.println("MQ9: Gas Detected!");
    air_quality_json["GasDetected"] = true;
  } else {
    Serial.println("MQ9: Air is Clean");
    air_quality_json["GasDetected"] = false;
  }

  MQ9.update();
  /*
  Exponential regression for each gas type (static values)
  GAS     | a      | b
  LPG     | 1000.5 | -2.186
  CH4     | 4269.6 | -2.648
  CO      | 599.65 | -2.244
  */

  MQ9.setA(1000.5); MQ9.setB(-2.186);
  float LPG = MQ9.readSensor(); // LPG ppm value

  MQ9.setA(4269.6); MQ9.setB(-2.648);
  float CH4 = MQ9.readSensor(); // CH4 ppm value
  
  MQ9.setA(599.65); MQ9.setB(-2.244);
  float CO = MQ9.readSensor(); // CO ppm value

  air_quality_json["LPG"] = LPG;
  air_quality_json["CH4"] = CH4;
  air_quality_json["CO"] = CO;
  int airQualityState = digitalRead(airSensorPinMQ135);
  
  if (airQualityState == LOW) {
    Serial.println("MQ135: Poor air quality detected!");
    air_quality_json["AirQuality"] = false;
  } else {
    Serial.println("MQ135: Air quality is okay.");
    air_quality_json["AirQuality"] = true;
  }
  serializeJson(air_quality_json, serializedString);
  Serial.println(serializedString);
  mqtt_client.publish_message(topic, serializedString);
  delay (1000);

  
  // if(gasState == LOW || airQualityState == LOW) digitalWrite(fanPin, HIGH); // Turn fan ON
  // else digitalWrite(fanPin, LOW);  // Turn fan OFF

  /*
  //Testing fan
  digitalWrite(fanPin, HIGH); // Turn fan ON
  delay(1000);
  digitalWrite(fanPin, LOW);  // Turn fan OFF
  delay(1000);
  */
  
}
