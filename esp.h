#ifndef ESP_H
#define ESP_H
#include <vector>
#include <string>
#include <cmath>
#include "cell.h"
#define ROW 70
#define COLUMN 70
using namespace std;

class ESP32
{
private:
    int x;
    int y;
    int number;
    string name;
    vector<Cell> cellgrid;
    void initializeCellGrid(int x, int y);
public:
    ESP32(int number, string name, int x, int y);
    Cell getCell(int x, int y);
    string getName();
    int getX();
    int getY();
};

#endif // ESP_H
