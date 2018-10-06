#include "dbmanager.h"
#include <thread>
#include "globals.h"


DbManager::DbManager(QObject *parent) : QThread(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("database.sqlite3");
}

DbManager::~DbManager()
{
    if(db.isOpen())
        db.close();
}

void DbManager::executeQuery(QListPacket packets,int row, int column)
{
    QSqlQuery query;
    int ESP32ID, EXPOS, EYPOS, RSSI;
    uint TIMESTAMP;
    string MAC, SSID, HASH;
    for(Packet p: packets)
    {
        ESP32ID = p.getESP32();
        EXPOS = p.getESP32_x();
        EYPOS = p.getESP32_y();
        TIMESTAMP = p.getTimestamp();
        RSSI = p.getRSSI();
        MAC = p.getMAC();
        SSID = p.getSSID();
        HASH = p.getHash();
        query.prepare("INSERT INTO packets (ESP, EXPOS, EYPOS, MAC, TIMESTAMP, RSSI, SSID, HASH, XPOS, YPOS) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
        query.addBindValue(ESP32ID);
        query.addBindValue(EXPOS);
        query.addBindValue(EYPOS);
        query.addBindValue(QString::fromStdString(MAC));
        query.addBindValue(TIMESTAMP);
        query.addBindValue(RSSI);
        query.addBindValue(QString::fromStdString(SSID));
        query.addBindValue(QString::fromStdString(HASH));
        query.addBindValue(column);
        query.addBindValue(row);
        query.exec();
    }
}

void DbManager::selectQuery(uint from, uint to)
{
    QSqlQuery query;
    QList<Packet> packets;
    query.prepare("SELECT * FROM packets WHERE TIMESTAMP <= ? AND TIMESTAMP >= ?");
    query.bindValue( 0, to );
    query.bindValue( 1, from );

    if( !query.exec() )
    {
        qDebug() << "Query Failed";
        return;
    }

    while( query.next() )
    {
        int esp32id = query.value( 0 ).toInt();
        int expos = query.value( 1 ).toInt();
        int eypos = query.value( 2 ).toInt();
        string mac = query.value( 3 ).toString().toStdString();
        uint timestamp = query.value( 4 ).toUInt();
        int rssi = query.value( 5 ).toInt();
        string ssid = query.value( 6 ).toString().toStdString();
        string hash = query.value( 7 ).toString().toStdString();
        int xpos = query.value( 8 ).toInt();
        int ypos = query.value( 9 ).toInt();
        if(expos == esp32devices.find(esp32id).value().getX() && eypos == esp32devices.find(esp32id).value().getY())
            packets.push_back(Packet(esp32id,expos,eypos,timestamp,rssi,mac,ssid,hash,xpos,ypos));
    }
    emit doUpdateGrid(packets);
}

void DbManager::run()
{
    if(!db.open())
    {
        return;
    }
    else
    {
        connect(this,SIGNAL(finished()),this,SLOT(quit()));
        exec();
    }

}
