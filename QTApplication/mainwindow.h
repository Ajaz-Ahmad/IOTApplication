#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDateTime>
#include "MQTTClient.h"
#include <QComboBox>
#include "sensor.h"
#define ADDRESS     "tcp://127.0.0.1:1883"
#define CLIENTID    "rpi2"
#define AUTHMETHOD  "ajazAhmad"
#define AUTHTOKEN   "test"
#define TOPIC       "ee513/cpuTemp"
#define PAYLOAD     53
#define QOS         1
#define TIMEOUT     10000L

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:
    QComboBox *selectTopic;
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_MQTTmessage(QString message);

    void on_comboBox_activated(const QString &arg1);

signals:
    void messageSignal(QString message);

private:
    Ui::MainWindow *ui;
    void update();
    int time;
    const char sensorTopic[100]="ee513/sensor/accelerometer";
    const char cpuTempTopic[100]="ee513/cpuTemp";
    const char* TopicName=this->sensorTopic;
    double temperature;
    MQTTClient client;
    Sensor accelerometer;
    volatile MQTTClient_deliveryToken deliveredtoken;

    friend void delivered(void *context, MQTTClient_deliveryToken dt);
    friend int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
    friend void connlost(void *context, char *cause);
};

void delivered(void *context, MQTTClient_deliveryToken dt);
int  msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message);
void connlost(void *context, char *cause);
int parseJSONData();
#endif // MAINWINDOW_H

