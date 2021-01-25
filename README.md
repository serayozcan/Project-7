# Project-7
It is the code for working fine with HC-SR04 and MLC9X0614 sensor using I2C. The code system has deterministic structure. Structure is correct, but you should check the synchronization parts for errors you receive. Good luck.
********************************************************

**Launchpad : EK-TM4C1294XL

**Environment : Code Composer Studio 10.2.0

**Connections 
HC-SR04 Ultrasonic Distance Sensor
5V – VCC
0V – GND
MLX90614 Single Infra Red Thermometer 
PD0 – SCL\n
PD1 – SDA \n
3.3V – VCC
0V – GND
Please make the connections right.

**Project Procedure
This project is about checking a person’s body temperature.
You have to use an ultrasonic range sensor to detect a person approaching.
When the person is less than 10 cm’s, you must trigger the task to measure the body temperature. 
If it is above 36,7, you will need to activate a red LED to indicate that there is a person with abnormal body temperature.
You must send the information (“person detected with ultrasonic range”, “person is within 30 cms”, “The person is within 10cm”, “triggerring temperature measurement”, “temperature measured is 36,3 C”, etc) to the Internet server. 

**Tools
To be able to use the code, please add timer and SWI via XGCONF Interface.
