#ifndef SENSOR_H
#define SENSOR_H


class Sensor
{
private:
    double pitch, roll;
public:
    Sensor();
    void setPitch(double);
    void setRoll(double);
    double getPitch();
    double getRoll();

};

#endif // SENSOR_H
