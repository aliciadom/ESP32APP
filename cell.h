#ifndef CELL_H
#define CELL_H
#include "packet.h"
using namespace std;
class Cell
{
private:
    double RSSI;
public:
    Cell(double rssi);
    double getrssi();
};

#endif // CELL_H
