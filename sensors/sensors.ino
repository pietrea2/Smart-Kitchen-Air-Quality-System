const int fanPin = D8;
const int airSensorPinMQ9 = D9;
const int airSensorPinMQ135 = D10;


void setup() {
  pinMode(fanPin, OUTPUT);
  pinMode(airSensorPinMQ9, INPUT);
  pinMode(airSensorPinMQ135, INPUT);
  Serial.begin(9600);
}

void loop() {

  int gasState = digitalRead(airSensorPinMQ9);
  if (gasState == LOW) {
    Serial.println("MQ9: Gas Detected!");
  } else {
    Serial.println("MQ9: Air is Clean");
  }


  int airQualityState = digitalRead(airSensorPinMQ135);
  if (airQualityState == LOW) {
    Serial.println("MQ135: Poor air quality detected!");
  } else {
    Serial.println("MQ135: Air quality is okay.");
  }
  
  if(gasState == LOW || airQualityState == LOW) digitalWrite(fanPin, HIGH); // Turn fan ON
  else digitalWrite(fanPin, LOW);  // Turn fan OFF

  /*
  //Testing fan
  digitalWrite(fanPin, HIGH); // Turn fan ON
  delay(1000);
  digitalWrite(fanPin, LOW);  // Turn fan OFF
  delay(1000);
  */
  
}
