#ifndef WORKER_H
#define WORKER_H
#include <QObject>
#include <QPoint>
#include <QPair>
#include <QMap>
#include <QList>
#include <QDateTime>
#include <packet.h>
#include <esp.h>
#include <thread>


typedef QList<QPair<Packet,QPoint>> QPackets;
typedef QMap<uint,QList<string>> QChartMap;
typedef QList<Packet> QListPacket;
typedef QMap<string,QListPacket> QMapHashPacket;

class Worker : public QObject {
    Q_OBJECT

public:
    Worker();
    ~Worker();

public slots:
    void doWorkSlot();
    void SetheckBoxIsChecked(bool checkBoxIsChecked);
    void SetQMapHashPacket(QMapHashPacket hashmap);
    void setAccuracy(int accuracy);
    void setUtil(int nesp32, int x_min,int x_max,int y_min,int y_max);
    void setEsp32Devices(QMap<int, ESP32> esp32devices);
signals:
    void resultReadySignal(QPackets packets, QChartMap chartmap, int distinctDevices);

private:
    bool isESP32Cell(int row, int column, QList<ESP32> esp32devices);
    bool inRange(int packet, int esp32, int accuracy);
    bool validBorder(int y, int x, int y_min, int y_max, int x_min, int x_max);
    bool checkBoxIsChecked;
    QMapHashPacket hashmap;
    QMap<int, ESP32> esp32devices;
    QMap<string,QPair<uint,QList<QPoint>>> recentMacPositionMap; // this map store the most recent position for a mac. mac<<time,places in the map>>
    int accuracy;
    int nesp32;
    int y_min;
    int x_min;
    int y_max;
    int x_max;

    QPackets packets;
    QList<string> distinctdevices;
    QMap<uint,QList<string>> chartmap;
    void job0(int x_start, int x_end, int y_start, int y_end);
    void job1(int x_start, int x_end, int y_start, int y_end);
};


#endif // WORKER_H