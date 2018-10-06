#include "esp.h"

ESP32::ESP32(int number, string name, int x, int y)
{
    this->name = name;
    this->number = number;
    this->x = x;
    this->y = y;
    initializeCellGrid(x,y);
}

void ESP32::initializeCellGrid(int x, int y)
{
    int j, i;
    for(j=0;j<ROW; j++)
    {
        for(i=0;i<COLUMN;i++)
        {
            if (i != x || j != y)
            {
                double d = sqrt(((i - x) * (i - x)) + ((j - y) * (j - y)));
                this->cellgrid.push_back(Cell(-round(d) - 34));
            }
            else
            {
                this->cellgrid.push_back(Cell(0));
            }
        }
    }
}

Cell ESP32::getCell(int x, int y)
{
    int pos = y*ROW + x;
    return this->cellgrid.at(static_cast<size_t>(pos));
}

string ESP32::getName()
{
    return this->name;
}
int ESP32::getX()
{
    return this->x;
}
int ESP32::getY()
{
    return this->y;
}
