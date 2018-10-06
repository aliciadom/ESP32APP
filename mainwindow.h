#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtDebug>
#include <QtCore>

#include <QString>
#include <fstream>
#include <esp.h>
#include <dbmanager.h>
using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void configureESP32Position();
    void setupGridLayout();
    void clearGridLayout();
    bool isESP32Cell(int row, int column);
    bool inRange(int packet, int esp32, int accuracy);
    bool validBorder(int y, int x, int y_min, int y_max, int x_min, int x_max);
signals:
    void querySignal(uint from, uint to,QListESP32 ESP32devices);
    void updateCellGridSignal(int row,int column,QString tooltip);
public slots:
   void handleButtonUpdateSlot();
   void updateGridLayoutSlot(QMapHashPacket);
   void updateCellGridSlot(int row,int column,QString tooltip);
private:
    Ui::MainWindow *ui;
    int nesp32;
    QMap<int, ESP32> esp32devices;
    int accuracy;
    int x_max, x_min, y_max, y_min;
    QList<string> distinctdevices;
    DbManager db;

};

#endif // MAINWINDOW_H
