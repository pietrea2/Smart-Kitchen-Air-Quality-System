import mqtt_client
import json

# Mosquitto (MQTT) configuration
ip_mosquitto = "192.168.0.27"
topic_mosquitto = "tester_topic"

# Connect to Mosquitto and subscribe to topics
# mqtt_client.message = None
rpi_mqtt_data = {"message": None, "topic": None}
mqttc = mqtt_client.connect(ip=ip_mosquitto, userdata=rpi_mqtt_data)
mqttc.subscribe(topic_mosquitto)

# Connecting to AWS
# mqtt_client.message = None
mqtt_broker_ip ="ao79tyrzu68h1-ats.iot.us-east-1.amazonaws.com"
mqtt_broker_port = 8883
mqtt_topic = "airquality/sensors"
ca_cert = "AmazonRootCA1.pem"
cert_file = "b1df023b51d2eba8c079bf93c81faae7904df8fc642b7e8660cf12da8a0f6213-certificate.pem.crt"
key_file = "b1df023b51d2eba8c079bf93c81faae7904df8fc642b7e8660cf12da8a0f6213-private.pem.key"
aws_mqtt = {"message": None, "topic": None}
mqtt_aws_client = mqtt_client.connect(mqtt_broker_ip, mqtt_broker_port, ca_cert, cert_file, key_file, aws_mqtt)
mqtt_aws_client.subscribe("fan_signal")
ENTRY_LIMIT = 10
print("MQTT Client connected")

metrics_to_calculate = ["CH4", "CO", "LPG"]
boolean_metrics = ["AirQuality", "GasDetected"]



def initialize_history_dict () -> dict:
    history_dict = {}
    for metric in metrics_to_calculate + boolean_metrics:
        history_dict[metric] = []

    history_dict["entries"] = 0
    return history_dict



def create_aws_dict (history_stored: dict):
    aws_dict = {}
    for metric in metrics_to_calculate:
        aws_dict[metric] = sum(history_stored[metric])/len(history_stored[metric])

    for metric in boolean_metrics:
        aws_dict[metric] = int(all(history_stored[metric]))
    return aws_dict



history_stored = initialize_history_dict()
# Keep checking if new data arrived
mqttc.loop_start()
mqtt_aws_client.loop_start()
print("MQTT Loop start")
while True:
    if rpi_mqtt_data["message"] is not None:
        # Retrieve topic and payload
        topic = rpi_mqtt_data["topic"]
        payload = str(rpi_mqtt_data["message"].payload.decode("utf-8"))

        payload_json = json.loads(payload)

        print(f"Entries: {history_stored['entries']}")
        print(f"sensor-controller - MQTT subscriber - Message received: {payload}")
        history_stored["entries"] += 1

        # Mapping all the required fields to the history dictionary

        for metric in metrics_to_calculate + boolean_metrics:
            history_stored[metric].append(payload_json[metric])


        # Only send the data to AWS IoT if we have 20 entries
        if history_stored["entries"] == ENTRY_LIMIT:
            print("Ready to Send to AWS")
            aws_dict = create_aws_dict(history_stored)
            mqtt_aws_client.publish(mqtt_topic, json.dumps(aws_dict))

            # Reset all the variables
            history_stored = initialize_history_dict()


        # Reset message
        rpi_mqtt_data["message"] = None
        rpi_mqtt_data["topic"] = None

    if aws_mqtt["message"] is not None:
        payload = str(aws_mqtt["message"].payload.decode("utf-8"))
        payload_json = json.loads(payload)
        aws_mqtt["message"] = None
        aws_mqtt["topic"] = None

        print(f"Received from Fan Topic in AWS: {payload_json}")

        if payload_json["fan_not_needed"] == 0:
            mqttc.publish("turn_fan", json.dumps({"fan": 1}))
        else:
            mqttc.publish("turn_fan", json.dumps({"fan": 0}))