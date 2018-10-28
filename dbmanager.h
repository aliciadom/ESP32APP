#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QtSql>
#include "packet.h"
#include "esp.h"
using namespace std;
typedef QList<Packet> QListPacket;
typedef QList<ESP32> QListESP32;
typedef QMap<string,QListPacket> QMapHashPacket;
class DbManager : public QThread
{
    Q_OBJECT
public:
    explicit DbManager(QObject *parent = nullptr);
    ~DbManager();
    void run();
    bool isESP32Cell(int row, int column, QList<ESP32> devices);
public slots:
    void querySlot(uint,uint, QListESP32);
signals:
    void updateGridSignal(QMapHashPacket);
private:
     QSqlDatabase db;

};

#endif // DBMANAGER_H
