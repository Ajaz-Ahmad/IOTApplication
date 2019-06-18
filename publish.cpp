// Based on the Paho C code example from www.eclipse.org/paho/
#include <iostream>
#include <sstream>
#include <fstream>
#include <string.h>
#include <unistd.h>
#include "MQTTClient.h"
#include "ADXL345.h"
#define  CPU_TEMP "/sys/class/thermal/thermal_zone0/temp"
using namespace std;
using namespace exploringRPi;

//Please replace the following address with the address of your server
#define ADDRESS    "tcp://192.168.0.46:1883"
#define CLIENTID   "rpi1"
#define AUTHMETHOD "ajazAhmad"
#define AUTHTOKEN  "test"
#define TOPIC      "ee513/"
#define QOS        0
#define TIMEOUT    10000L


float getCPUTemperature() {        // get the CPU temperature
   int cpuTemp;                    // store as an int
   fstream fs;
   fs.open(CPU_TEMP, fstream::in); // read from the file
   fs >> cpuTemp;
   fs.close();
   return (((float)cpuTemp)/1000);
}

ADXL345 getPitchAndRollData(){
	ADXL345 sensor(1,0x53);
	sensor.setResolution(ADXL345::NORMAL);
	sensor.setRange(ADXL345::PLUSMINUS_4_G);
	sensor.readSensorState();
	return sensor;
}

int publishCPUTemperature(){
   char temp_payload[100];         // Set your max message size here
   MQTTClient client;
   MQTTClient_deliveryToken token;
   MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
   MQTTClient_willOptions will_opts=MQTTClient_willOptions_initializer;
   will_opts.topicName="ee513/cpuTemp";
   will_opts.message="Lost Connection";
   will_opts.retained=0;
   will_opts.qos=1;
   opts.will=&will_opts;
   MQTTClient_message pubmsg = MQTTClient_message_initializer;
   MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
   opts.keepAliveInterval = 20;
   opts.cleansession = 1;
   opts.username = AUTHMETHOD;
   opts.password = AUTHTOKEN;
   int rc;
   if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
      cout << "Failed to connect, return code " << rc << endl;
      return -1;
   }
   sprintf(temp_payload, "{\"CPU\":{\"CPUTemp\": %f}}", getCPUTemperature()); 
   pubmsg.payload = temp_payload;
   pubmsg.payloadlen = strlen(temp_payload);
   pubmsg.qos = 2;
   pubmsg.retained = 0;
   char topic[100]="ee513/cpuTemp";
   MQTTClient_publishMessage(client, topic, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << temp_payload <<
        " \non topic " << topic << " for ClientID: " << CLIENTID << endl; 
   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   cout << "Message with token " << (int)token << " delivered."<<" RC "<< rc<< endl;
return rc;
}

int publishSensorData(){
   char sensor_payload[100];         // Set your max message size here
   MQTTClient client;
   MQTTClient_deliveryToken token;
   MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
   MQTTClient_willOptions will_option=MQTTClient_willOptions_initializer;
   will_option.topicName="ee513/sensor/accelerometer";
   will_option.message="Lost Connection for Accelerometer";
   will_option.retained=0;
   will_option.qos=1;
   opts.will=&will_option;
   MQTTClient_message pubmsg = MQTTClient_message_initializer;
   char clientId[10]="rp";
   MQTTClient_create(&client, ADDRESS, clientId, MQTTCLIENT_PERSISTENCE_NONE, NULL);
   opts.keepAliveInterval = 20;
   opts.cleansession = 1;
   opts.username = AUTHMETHOD;
   opts.password = AUTHTOKEN;
   int rc;
   if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
      cout << "Failed to connect, return code " << rc << endl;
      return -1;
   }
   ADXL345 sensor=getPitchAndRollData();
   sprintf(sensor_payload, "{\"Pitch\":%f, \"Roll\":%f }}", sensor.getPitch(), sensor.getRoll()); 
   pubmsg.payload = sensor_payload;
   pubmsg.payloadlen = strlen(sensor_payload);
   pubmsg.qos = 0;
   pubmsg.retained = 0;
   char Topic[100]="ee513/sensor/accelerometer";
   MQTTClient_publishMessage(client, Topic, &pubmsg, &token);
   cout << "Waiting for up to " << (int)(TIMEOUT/1000) <<
        " seconds for publication of " << sensor_payload <<
        " \non topic " << Topic << " for ClientID: " << CLIENTID << endl; 
   rc = MQTTClient_waitForCompletion(client, token, TIMEOUT);
   cout << "Message with token " << (int)token << " delivered."<<" RC "<< rc<< endl;
return rc;
}
int main(int argc, char* argv[]) {
  while(true){
	publishSensorData();
	publishCPUTemperature();
	usleep(10000000);
}
   return 0;
}


