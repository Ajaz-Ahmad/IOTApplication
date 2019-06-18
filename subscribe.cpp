#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <string>
#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include "MQTTClient.h"
#include <wiringPi.h>
#define ADDRESS     "tcp://192.168.0.46:1883"
#define CLIENTID    "rpi2"
#define AUTHMETHOD  "ajazAhmad"
#define AUTHTOKEN   "test"
#define TOPIC       "ee513/cpuTemp"
#define PAYLOAD     "Recevied cpu data"
#define QOS         1
#define TIMEOUT     10000L

using namespace std;
volatile MQTTClient_deliveryToken deliveredtoken;

void delivered(void *context, MQTTClient_deliveryToken dt) {
    printf("Message with token value %d delivery confirmed\n", dt);
    deliveredtoken = dt;
}
void turnOnLED(){
       wiringPiSetup () ;
       pinMode (7, OUTPUT) ;
         digitalWrite (7, HIGH);
}
void turnOffLED(){
	wiringPiSetup();
	pinMode(7,OUTPUT);
	digitalWrite(7,LOW);
}
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    int i;
    char* payloadptr;
    string valueTemp="";
    Json::Reader reader;
    Json::Value obj;
    printf("Message arrived\n");
    printf("topic: %s\n", topicName);
    printf("message: ");
	bool b=reader.parse((char*)message->payload,obj);
	if(b!=0){
		cout<<"Value is "<<obj["CPU"]<<endl;
		for(Json::Value::iterator it=obj.begin();it!=obj.end();++it){
			valueTemp=(*it)["CPUTemp"].asString();
		}
		double temperature=::atof(valueTemp.c_str());
		cout<<"Temperature is:"<<temperature<<endl;
		if(temperature>55){
			turnOnLED();
		}
		else
			turnOffLED();
	}
	else{
		payloadptr = (char*) message->payload;
   		for(i=0; i<message->payloadlen; i++) {
		   	putchar(*payloadptr++);	
		}
	turnOffLED();
    }
putchar('\n');
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

void connlost(void *context, char *cause) {
    printf("\nConnection lost\n");
    printf("     cause: %s\n", cause);
}

int main(int argc, char* argv[]) {
    MQTTClient client;
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    int ch;

    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 1;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    printf("Subscribing to topic %s\nfor client %s using QoS%d\n\n"
           "Press Q<Enter> to quit\n\n", TOPIC, CLIENTID, QOS);
    MQTTClient_subscribe(client, TOPIC, QOS);

    do {
        ch = getchar();
    } while(ch!='Q' && ch != 'q');
    MQTTClient_disconnect(client, 10000);
    MQTTClient_destroy(&client);
    return rc;
}

