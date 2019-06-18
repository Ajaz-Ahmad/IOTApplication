#include "sensor.h"

Sensor::Sensor()
{
    this->pitch=0;
    this->roll=0;
}
void Sensor::setPitch(double pitch){
    this->pitch=pitch;
}
void Sensor::setRoll(double roll){
    this->roll=roll;
}
double Sensor::getPitch(){
    return this->pitch;
}
double Sensor::getRoll(){
    return this->roll;
}
