import mqtt_client
import json

# Mosquitto (MQTT) configuration
ip_mosquitto = "192.168.2.214"
topic_mosquitto = "tester_topic"

# Connect to Mosquitto and subscribe to topics
mqtt_client.message = None
mqttc = mqtt_client.connect(ip_mosquitto)
mqttc.subscribe(topic_mosquitto)

# Connecting to AWS
mqtt_client.message = None
mqtt_broker_ip ="ao79tyrzu68h1-ats.iot.us-east-1.amazonaws.com"
mqtt_broker_port = 8883
mqtt_topic = "airquality/sensors"
ca_cert = "AmazonRootCA1.pem"
cert_file = < FILE NAME ENDING IN CERTIFICATE PEM CRT>
key_file = <FILE NAME ENDING WITH PRIVATE PEM CRT>

mqtt_aws_client = mqtt_client.connect(mqtt_broker_ip, mqtt_broker_port, ca_cert, cert_file, key_file)

ENTRY_LIMIT = 10

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
while True:
    if mqtt_client.message is not None:
        # Retrieve topic and payload
        topic = mqtt_client.message
        payload = str(mqtt_client.message.payload.decode("utf-8"))

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
        mqtt_client.message = None

