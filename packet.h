#ifndef PACKET_H
#define PACKET_H
#include <string>
using namespace std;
class Packet
{
private:
    int ESP32;
    int ESP32_X;
    int ESP32_Y;
    uint TIMESTAMP;
    int RSSI;
    string MAC;
    string SSID;
    string HASH;
    int x_pos = -1;
    int y_pos = -1;
public:
    Packet(int ESP32 ,int ESP32_X, int ESP32_Y, uint TIMESTAMP,int RSSI, string MAC, string SSID, string HASH);
    Packet(int ESP32 ,int ESP32_X, int ESP32_Y, uint TIMESTAMP,int RSSI, string MAC, string SSID, string HASH, int X, int Y);
    int getESP32();
    int getESP32_x();
    int getESP32_y();
    uint getTimestamp();
    int getRSSI();
    string getMAC();
    string getSSID();
    string getHash();
    string toString();
    int getX();
    int getY();
    void setX(int);
    void setY(int);
};

#endif // PACKET_H
