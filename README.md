# UofT ECE1528 Final Project: Smart Kitchen Air Quality Monitoring and Controlling System

Created by:
- Divya Sivakumaran
- Adam Pietrewicz

## Project Description 
The proposed project is a smart kitchen air quality system that a user can install in their home kitchen to 
monitor the air quality status wirelessly through a web application. The system comprises portable air 
quality sensor modules that can be placed around the desired room (even installed on an oven air vent) 
which connect to a cloud server and upload their air quality readings through an edge device. This data 
will be analyzed and displayed to the user on a computer application where they can read their air quality 
status from each sensor module. In addition, the system includes fans that are placed within the kitchen 
and automatically controlled by the system depending on the detection of harmful gases or smoke. For the 
scope of this project, a prototype was developed to show the feasibility of the produc

![Arduino Air QUality Sensor Module](https://github.com/pietrea2/Smart-Kitchen-Air-Quality-System/tree/main/diagrams/arduino_module.png)

Main System Components:
- sensors and actuators (gas/smoke sensor, DC fan)
- communication through edge device/gateway (between Arduino and Raspberry PI)
- cloud server (AWS) and user dashboard


