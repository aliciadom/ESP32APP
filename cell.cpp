#include "cell.h"

Cell::Cell(double RSSI)
{
    this->RSSI = RSSI;

}
double Cell::getrssi()
{
    return this->RSSI;
}
