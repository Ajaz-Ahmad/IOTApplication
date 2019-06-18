#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QDebug>
#include<iostream>
using namespace std;
MainWindow *handle;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    selectTopic=new QComboBox();
    selectTopic->addItem("ee513/cpuTemperature");
    selectTopic->addItem("ee513/sensorData/accelerometer");
    this->temperature=0;
    this->time = 0;
    this->setWindowTitle("EE513 Assignment 2");
    this->ui->customPlot->addGraph();
    this->ui->comboBox->addItem("ee513/sensor/accelerometer");
    this->ui->comboBox->addItem("ee513/cpuTemp");
    this->ui->customPlot->yAxis->setLabel("Pitch (degrees)");
    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    this->ui->customPlot->xAxis->setTicker(timeTicker);
    this->ui->customPlot->yAxis->setRange(-100,100);
    this->ui->customPlot->replot();
    QObject::connect(this, SIGNAL(messageSignal(QString)),
                     this, SLOT(on_MQTTmessage(QString)));
    ::handle = this;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(){
    static QTime time(QTime::currentTime());
    double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
    QString name=this->TopicName;
    QString receivedTopic="ee513/cpuTemp";
    if(name.contains(receivedTopic)){
        ui->customPlot->graph(0)->addData(key,temperature);
        ui->customPlot->graph(0)->rescaleKeyAxis(true);
        ui->customPlot->replot();
        QString text= QString("Value added is %1").arg(this->temperature);
        ui->outputEdit->setText(text);
    }
    else{
        ui->customPlot->graph(0)->addData(key,this->accelerometer.getPitch());
        this->ui->customPlot->yAxis->setRange(-100,100);
        ui->customPlot->graph(0)->rescaleKeyAxis(true);
        ui->customPlot->replot();
        cout<<"Accel:"<<this->accelerometer.getPitch()<<endl;
        QString text= QString("Value added is %1").arg(this->accelerometer.getPitch());
        ui->outputEdit->setText(text);
    }
}

double parseJSONData(QString str){
    QJsonDocument doc=QJsonDocument::fromJson((str.toUtf8()));
    QJsonObject obj=doc.object();
    QJsonObject sample=obj["CPU"].toObject();
    double temperature=sample["CPUTemp"].toDouble();
    cout<<"Temp"<<temperature<<endl;
    return temperature;
}
Sensor parseSensorData(QString str){
    Sensor sensor;
    QJsonDocument doc=QJsonDocument::fromJson((str.toUtf8()));
    QJsonObject obj=doc.object();
    QJsonObject sample=obj["Accelerometer"].toObject();
    cout<<"dsfd"<<sample["Pitch"].toDouble()<<endl;
    sensor.setPitch(sample["Pitch"].toDouble());
    sensor.setRoll(sample["Roll"].toDouble());
    return sensor;
}
void MainWindow::on_connectButton_clicked()
{
    MQTTClient_connectOptions opts = MQTTClient_connectOptions_initializer;
    int rc;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_DEFAULT, NULL);
    opts.keepAliveInterval = 20;
    opts.cleansession = 0;
    opts.username = AUTHMETHOD;
    opts.password = AUTHTOKEN;

    if (MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered)==0){
        ui->outputText->appendPlainText(QString("Callbacks set correctly"));
    }
    if ((rc = MQTTClient_connect(client, &opts)) != MQTTCLIENT_SUCCESS) {
        ui->outputText->appendPlainText(QString("Failed to connect, return code %1").arg(rc));
    }
    ui->outputText->appendPlainText(QString("Subscribing to topic " TOPIC " for client " CLIENTID));

    int x = MQTTClient_subscribe(client, this->TopicName ,QOS);
    ui->outputText->appendPlainText(QString("Result of subscribe is %1 (0=success)").arg(x));
}

void delivered(void *context, MQTTClient_deliveryToken dt) {
    (void)context;
    // Please don't modify the Window UI from here
    qDebug() << "Message delivery confirmed";
    handle->deliveredtoken = dt;
}

/* This is a callback function and is essentially another thread. Do not modify the
 * main window UI from here as it will cause problems. Please see the Slot method that
 * is directly below this function. To ensure that this method is thread safe I had to
 * get it to emit a signal which is received by the slot method below */
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    (void)context; (void)topicLen;
    qDebug() << "Message arrived (topic is " << topicName << ")";
    qDebug() << "Message payload length is " << message->payloadlen;
    QString payload;
    payload.sprintf("%s", (char *) message->payload).truncate(message->payloadlen);
    emit handle->messageSignal(payload);
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

/** This is the slot method. Do all of your message received work here. It is also safe
 * to call other methods on the object from this point in the code */
void MainWindow::on_MQTTmessage(QString payload){
    string topicNames="";

    ui->outputText->ensureCursorVisible();
    QString name=this->TopicName;
    QString receivedTopic="ee513/cpuTemp";
    if(name.contains(receivedTopic)){
        this->temperature=parseJSONData(payload);
        QString value="Temperature received"+QString::number(temperature);
        ui->outputText->appendPlainText(value);
    }
    else{
        this->accelerometer=parseSensorData(payload);
        QString value="Temperature received"+QString::number(this->accelerometer.getPitch());
        ui->outputText->appendPlainText(value);
    }
    this->update();
}

void connlost(void *context, char *cause) {
    (void)context; (void)*cause;
    // Please don't modify the Window UI from here
    qDebug() << "Connection Lost" << endl;
}

void MainWindow::on_disconnectButton_clicked()
{
    qDebug() << "Disconnecting from the broker" << endl;
    MQTTClient_disconnect(client, 10000);
    //MQTTClient_destroy(&client);
}

void MainWindow::on_comboBox_activated(const QString &arg1)
{
    QString textTopic=this->ui->comboBox->currentText();
    QString topicSelected="ee513/cpuTemp";
    if(textTopic.contains(topicSelected)){
        this->ui->customPlot->yAxis->setLabel("Temperature (degrees)");
        this->TopicName=this->cpuTempTopic;
        this->update();
    }
    else{
        this->ui->customPlot->yAxis->setLabel("Pitch (degrees)");
        this->TopicName=this->sensorTopic;
        this->update();
    }
}
