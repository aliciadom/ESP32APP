#include "packet.h"

Packet::Packet(int ESP32,int ESP32_X, int ESP32_Y,uint TIMESTAMP,int RSSI, string MAC, string SSID, string HASH)
{
    this->ESP32 = ESP32;
    this->ESP32_X = ESP32_X;
    this->ESP32_Y = ESP32_Y;
    this->TIMESTAMP = TIMESTAMP;
    this->RSSI = RSSI;
    this->MAC = MAC;
    this->SSID = SSID;
    this->HASH = HASH;
}
Packet::Packet(int ESP32,int ESP32_X, int ESP32_Y,uint TIMESTAMP,int RSSI, string MAC, string SSID, string HASH, int X, int Y)
{
    this->ESP32 = ESP32;
    this->ESP32_X = ESP32_X;
    this->ESP32_Y = ESP32_Y;
    this->TIMESTAMP = TIMESTAMP;
    this->RSSI = RSSI;
    this->MAC = MAC;
    this->SSID = SSID;
    this->HASH = HASH;
    this->x_pos = X;
    this->y_pos = Y;
}
int Packet::getESP32()
{
    return this->ESP32;
}
int Packet::getESP32_x()
{
    return this->ESP32_X;
}
int Packet::getESP32_y()
{
    return this->ESP32_Y;
}
uint Packet::getTimestamp()
{
    return this->TIMESTAMP;
}
int Packet::getRSSI()
{
    return this->RSSI;
}
string Packet::getMAC()
{
    return this->MAC;
}
string Packet::getSSID()
{
    return this->SSID;
}
string Packet::getHash()
{
    return this->HASH;
}

int Packet::getX()
{
    return this->x_pos;
}
int Packet::getY()
{
    return this->y_pos;
}

void Packet::setX(int x)
{
    this->x_pos = x;
}

void Packet::setY(int y)
{
    this->y_pos = y;
}

string Packet::toString()
{
    string s = "";
    s.append(to_string(this->ESP32)).
            append(" ").
            append(to_string(this->ESP32_X)).
            append(" ").
            append(to_string(this->ESP32_Y)).
            append(" ").
            append(to_string(this->TIMESTAMP)).
            append(" ").
            append(to_string(this->RSSI)).
            append(" ").
            append(this->MAC).
            append(" ").
            append(this->SSID).
            append(" ").
            append(this->HASH).
            append(" ").
            append(to_string(this->x_pos)).
            append(" ").
            append(to_string(this->y_pos));
    return s;
}
