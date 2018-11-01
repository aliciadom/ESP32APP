#include "dbmanager.h"

DbManager::DbManager(QObject *parent) : QThread(parent)
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    //db.setDatabaseName("database.sqlite3");
    db.setDatabaseName("sniff.db");
}

DbManager::~DbManager()
{
    if(db.isOpen())
        db.close();
}

void DbManager::querySlot(uint from, uint to, QListESP32 esp32devices)
{
    QSqlQuery query;
    QMapHashPacket map;
    query.prepare("SELECT ESPID,X,Y,MAC,TIMESTAMP,RSSI,SSID FROM SNIFF WHERE (TIMESTAMP,MAC) IN"
                  "(SELECT TIMESTAMP,MAC FROM SNIFF"
                  " WHERE TIMESTAMP <= ? AND TIMESTAMP >= ?"
                  " GROUP BY TIMESTAMP,MAC"
                  " HAVING COUNT(DISTINCT ESPID)=?)");
    query.bindValue( 0, to );
    query.bindValue( 1, from );
    query.bindValue( 2, esp32devices.count() );

    if( !query.exec() )
    {
        qDebug() << "Query Failed";
        return;
    }

    qDebug() << from;
    qDebug() << to;

    while( query.next() )
    {
        int esp32id = query.value( 0 ).toInt()-1;
        int expos = query.value( 1 ).toInt();
        int eypos = query.value( 2 ).toInt();
        string mac = query.value( 3 ).toString().toStdString();
        string tmstmp = query.value( 4 ).toString().toStdString();
        uint timestamp = query.value( 4 ).toUInt();
        int rssi = query.value( 5 ).toInt();
        string ssid = query.value( 6 ).toString().toStdString();
        string hash = Packet::generateHash(mac+tmstmp);
        if(esp32devices.length()>esp32id)
        {
            ESP32 e = esp32devices.at(esp32id);
            if(expos == e.getX() && eypos == e.getY())
            {
                Packet p(esp32id,expos,eypos,timestamp,rssi,mac,ssid,hash);
                if(!map.contains(hash))
                {
                    QListPacket list;
                    list.push_back(p);
                    map.insert(hash,list);
                    //qDebug() << QString::fromStdString(p.toString());
                }
                else
                {

                    bool flag = true;
                    //make sure to insert a packet(esp32,hash) only one time
                    for(Packet pck : map[hash])
                    {
                        int esp32id = pck.getESP32();
                        if(esp32id == p.getESP32())
                        {
                            //skip this packet
                            flag = false;
                        }
                    }
                    if(flag)
                    {
                        map[hash].push_back(p);
                        //qDebug() << QString::fromStdString(p.toString());
                    }


                }
            }
        }
    }
    emit updateGridSignal(map);
}

void DbManager::run()
{
    if(!db.open())
    {
        qDebug() << "Impossible open db";
        return;
    }
    else
    {
        qDebug() << "db opend";
        connect(this,SIGNAL(finished()),this,SLOT(quit()));
        exec();
    }

}

bool DbManager::isESP32Cell(int row, int column, QList<ESP32> devices)
{
    for(auto value : devices)
    {
        if(value.getX() == column && value.getY() == row)
        {
            return true;
        }
    }
    return false;
}
