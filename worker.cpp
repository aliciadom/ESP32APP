#include "worker.h"
#include <QDebug>

Worker::Worker(){
}

Worker::~Worker() {
}

void Worker::job0(int x_start, int x_end, int y_start, int y_end){
    qDebug() << "only recent pos";
    for (int i = y_start; i < y_end; i++)
    {
        for(int j = x_start; j< x_end; j++)
        {
            if(!isESP32Cell(i,j,esp32devices.values()))
            {
                QString s;
                for (string hash : hashmap.keys())
                {
                    int count = 0;
                    for(Packet p : hashmap[hash])
                    {
                        int esp32id = p.getESP32();
                        ESP32 esp32 = esp32devices.find(esp32id).value();
                        double esp32rssi = esp32.getCell(j,i).getrssi();
                        if(inRange(p.getRSSI(),static_cast<int>(esp32rssi),accuracy) && validBorder(i,j,y_min,y_max,x_min,x_max))
                        {
                            count++;
                        }
                    }
                    if(count == nesp32)
                    {
                        Packet p = hashmap[hash][0];
                        string packethash = p.getHash();
                        string mac = p.getMAC();
                        uint unixtime = p.getTimestamp();
                        if(!recentMacPositionMap.contains(mac))
                        {
                                QList<QPoint> list;
                                QPoint qpoint(j,i);
                                list.push_back(qpoint);
                                QPair<uint,QList<QPoint>> qpair;
                                qpair.first = unixtime;
                                qpair.second = list;
                                recentMacPositionMap.insert(mac,qpair);
                        }
                        else
                        {
                            QPair<uint,QList<QPoint>> qpair = recentMacPositionMap.value(mac);
                            if(qpair.first<unixtime)
                            {
                                recentMacPositionMap.remove(mac);
                                QList<QPoint> list;
                                QPoint qpoint(j,i);
                                list.push_back(qpoint);
                                QPair<uint,QList<QPoint>> nqpair;
                                nqpair.first = unixtime;
                                nqpair.second = list;
                                recentMacPositionMap.insert(mac,nqpair);
                            }
                            else if(qpair.first==unixtime)
                            {
                                  QPoint qpoint(j,i);
                                  recentMacPositionMap[mac].second.push_back(qpoint);
                            }
                        }
                    }
                }
            }
        }
    }



    for(string s : recentMacPositionMap.keys())
    {
        QPair<uint,QList<QPoint>> qpair = recentMacPositionMap.value(s);
        string hash = Packet::generateHash(s+to_string(qpair.first));
        Packet p = hashmap.value(hash)[0];
        uint unixtime = p.getTimestamp();
        QDateTime time;
        time.setTime_t(unixtime);




        for(QPoint qp : qpair.second)
        {
            QPair<Packet,QPoint> pairvalue = qMakePair(p,qp);;
            packets.push_back(pairvalue);

        }

        if(!chartmap.contains(unixtime))
        {
            QList<string> list;
            list.append(s);
            chartmap.insert(unixtime,list);
        }
        else
        {
            QList<string> list = chartmap.value(unixtime);
            if(!list.contains(s))
                list.append(s);
        }
        if(!distinctdevices.contains(hash))
        {
            distinctdevices.push_back(hash);
        }
    }
}
void Worker::job1(int x_start, int x_end, int y_start, int y_end){
    qDebug() << "not only recent pos";
    for (int i = y_start; i < y_end; i++)
    {
        for(int j = x_start; j< x_end; j++)
                    {
                        if(!isESP32Cell(i,j,esp32devices.values()))
                        {
                            QString s;
                            for (string hash : hashmap.keys())
                            {
                                int count = 0;
                                for(Packet p : hashmap[hash])
                                {
                                    int esp32id = p.getESP32();
                                    ESP32 esp32 = esp32devices.find(esp32id).value();
                                    double esp32rssi = esp32.getCell(j,i).getrssi();
                                    if(inRange(p.getRSSI(),static_cast<int>(esp32rssi),accuracy) && validBorder(i,j,y_min,y_max,x_min,x_max))
                                    {
                                        count++;
                                    }
                                }
                                if(count == nesp32)
                                {
                                    Packet p = hashmap[hash][0];
                                    string packethash = p.getHash();
                                    string mac = p.getMAC();
                                    uint unixtime = p.getTimestamp();


                                    QPoint qp(j,i);
                                    QPair<Packet,QPoint> pairvalue = qMakePair(p,qp);;
                                    packets.push_back(pairvalue);

                                    if(!distinctdevices.contains(packethash))
                                    {
                                        distinctdevices.push_back(packethash);
                                    }
                                    if(!chartmap.contains(unixtime))
                                    {
                                        QList<string> list;
                                        list.append(mac);
                                        chartmap.insert(unixtime,list);
                                    }
                                    else
                                    {
                                        QList<string> list = chartmap.value(unixtime);
                                        if(!list.contains(mac))
                                            list.append(mac);

                                    }
                                }
                            }
                        }
                    }
                }
}


void Worker::doWorkSlot() {

    qDebug() << "here the work accuracy,xmin,xmax,ymin,ymax = " + QString::number(accuracy) + QString::number(x_min) + QString::number(x_max) +QString::number(y_min) + QString::number(y_max);

    packets.clear();
    distinctdevices.clear();
    chartmap.clear();
    if(checkBoxIsChecked)
    {
       job0(x_min+1,x_max,y_min+1,y_max);

    }
    else
    {
        job1(x_min+1,x_max,y_min+1,y_max);
    }
    qDebug() << distinctdevices.count();
    emit resultReadySignal(packets, chartmap, distinctdevices.count());
}

bool Worker::isESP32Cell(int row, int column, QList<ESP32> esp32devices)
{
    for(auto value : esp32devices)
    {
        if(value.getX() == column && value.getY() == row)
        {
            return true;
        }
    }
    return false;
}
bool Worker::inRange(int packet, int esp32, int accuracy)
{
    return (abs(packet - esp32) <= accuracy);
}
bool Worker::validBorder(int y, int x, int y_min, int y_max, int x_min, int x_max)
{
    return (y > y_min && y < y_max && x > x_min && x < x_max);
}

void Worker::setAccuracy(int accuracy)
{
    this->accuracy = accuracy;
}

void Worker::SetheckBoxIsChecked(bool checkBoxIsChecked)
{
    this->checkBoxIsChecked = checkBoxIsChecked;
    qDebug() << checkBoxIsChecked;
}

void Worker::SetQMapHashPacket(QMapHashPacket hashmap)
{
    this->hashmap = hashmap;
    qDebug() << "fuck";
}

void Worker::setUtil(int nesp32, int x_min,int x_max,int y_min,int y_max){
    this->nesp32 = nesp32;
    this->x_max = x_max;
    this->x_min = x_min;
    this->y_max = y_max;
    this->y_min = y_min;
}
void Worker::setEsp32Devices(QMap<int, ESP32> esp32devices){
    this->esp32devices = esp32devices;
}
