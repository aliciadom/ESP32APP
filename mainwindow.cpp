#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    configureESP32Position();
    setupGridLayout();
    ui->dateTimeFrom->setDate(QDate::currentDate());
    ui->dateTimeTo->setDate(QDate::currentDate());
    connect(ui->buttonUpdate, SIGNAL (released()), this, SLOT (handleButtonUpdateSlot()));
    connect(this,SIGNAL(querySignal(uint,uint,QListESP32)), &db, SLOT(querySlot(uint,uint,QListESP32)));
    connect(&db, SIGNAL(updateGridSignal(QMapHashPacket)), this, SLOT(updateGridLayoutSlot(QMapHashPacket)));
    connect(this,SIGNAL(updateCellGridSignal(int, int, QString)),this,SLOT(updateCellGridSlot(int, int, QString)));
    db.start();
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::configureESP32Position()
{
    ifstream infile;
    QString line;
    int x_coord;
    int y_coord;

    QFile file("config.txt");
    if(file.exists())
    {
        if (file.open(QIODevice::ReadOnly | QIODevice::Text))
        {
           QTextStream in(&file);
           line = in.readLine();
           nesp32 = stoi(line.toUtf8().constData());
           for(int i = 0; i<nesp32; i++)
           {
              line = in.readLine();
              QStringList list = line.split(QRegExp("\\s"));
              x_coord = stoi(list.first().toUtf8().constData());
              y_coord = stoi(list.last().toUtf8().constData());
              qDebug() << x_coord;
              qDebug() << y_coord;
              ESP32 esp32(0,"ESP"+to_string(i),x_coord,y_coord);
              esp32devices.insert(i,esp32);
              if(x_coord > x_max) x_max = x_coord;
              if(y_coord > y_max) y_max = y_coord;
              if(x_coord < x_min) x_min = x_coord;
              if(y_coord < y_min) y_min = y_coord;
           }
           line = in.readLine();
           accuracy = stoi(line.toUtf8().constData());
           qDebug() << accuracy;
           file.close();
        }
        else
        {
            qInfo("File not open");
        }
    }
    else
    {
        qInfo("File does not exist");
    }
}
void MainWindow::setupGridLayout()
{
    QPalette empty, espcolor;
    empty.setColor(QPalette::Window, Qt::yellow);
    espcolor.setColor(QPalette::Window, Qt::black);
    for (int i = 0; i < ROW; i++)
    {
        for(int j = 0; j< COLUMN; j++)
        {
            QLabel *ql = new QLabel(centralWidget());
            QString s;
            s.append("POS[X,Y]: [").append(QString::number(j)).append(",").append(QString::number(i)).append("]\n");
            ql->setAutoFillBackground(true);
            ql->setFrameShape(QFrame::StyledPanel);
            ql->setPalette(empty);
            for (ESP32 esp32 : esp32devices)
            {
                double value = esp32.getCell(j,i).getrssi();
                string esp32name = esp32.getName();
                s.append(QString::fromUtf8(esp32name.c_str())).append(" RSSI: ").append(QString::number(value)).append("\n");
                if(value==0.0)
                {
                    ql->setPalette(espcolor);
                }
            }
            ql->setToolTip(s);
            ql->setFixedSize(QSize(20,20));
            ui->gridLayout->addWidget(ql,i,j);
            ui->gridLayout->setSpacing(0);
        }
    }
}
void MainWindow::clearGridLayout()
{
    qDebug() << "clearGridLayout()..";
    QPalette empty;
    empty.setColor(QPalette::Window, Qt::yellow);

    for (int i = 0; i < ROW; i++)
    {
        for(int j = 0; j< COLUMN; j++)
        {
            QWidget *qw = ui->gridLayout->itemAtPosition(i,j)->widget();
            QColor qcolor = qw->palette().color(QPalette::Window);
            if(qcolor==Qt::red)
            {
                QString s;
                s.append("POS[X,Y]: [").append(QString::number(j)).append(",").append(QString::number(i)).append("]\n");
                for (ESP32 esp32 : esp32devices)
                {
                    double value = esp32.getCell(j,i).getrssi();
                    string esp32name = esp32.getName();
                    s.append(QString::fromUtf8(esp32name.c_str())).append(" RSSI: ").append(QString::number(value)).append("\n");
                }
                qw->setToolTip("");
                qw->setPalette(empty);
            }
        }
    }
}
void MainWindow::handleButtonUpdateSlot()
{
    QDateTime from = ui->dateTimeFrom->dateTime();
    QDateTime to = ui->dateTimeTo->dateTime();
    uint unixtimefrom = from.toTime_t();
    uint unixtimeto = to.toTime_t();
    clearGridLayout();

    ui->deviceslabel->setText(QString::number(0));
    distinctdevices.clear();
    emit querySignal(unixtimefrom,unixtimeto,esp32devices.values());
}
void MainWindow::updateGridLayoutSlot(QMapHashPacket hashmap)
{
    qDebug() << "updateGridLayoutSlot()..";
    distinctdevices.clear();
    if(!hashmap.isEmpty())
    {
        for (int i = 0; i < ROW; i++)
        {
            for(int j = 0; j< COLUMN; j++)
            {
                if(!isESP32Cell(i,j))
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
                            uint unixtime = p.getTimestamp();
                            QDateTime time;
                            time.setTime_t(unixtime);
                            s.append("{\n\tMAC: ").append(QString::fromUtf8(p.getMAC().c_str())).append("\n\t:").append(time.toString()).append("\n}\n");
                            if(!distinctdevices.contains(packethash))
                                distinctdevices.push_back(packethash);
                            emit updateCellGridSignal(i,j,s);
                        }
                    }
                }

            }
        }
    }
    ui->deviceslabel->setText(QString::number(distinctdevices.count()));
}
void MainWindow::updateCellGridSlot(int row, int column, QString tooltip)
{
    QPalette qp;
    qp.setColor(QPalette::Window, Qt::red);
    QWidget *qw = ui->gridLayout->itemAtPosition(row,column)->widget();
    QString qwstring = qw->toolTip();
    QPalette qwpalette = qw->palette();
    qwstring.append(tooltip);
    qw->setToolTip(qwstring);
    qw->setPalette(qp);
}
bool MainWindow::isESP32Cell(int row, int column)
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
bool MainWindow::inRange(int packet, int esp32, int accuracy)
{
    return (abs(packet - esp32) <= accuracy);
}
bool MainWindow::validBorder(int y, int x, int y_min, int y_max, int x_min, int x_max)
{
    return (y >= y_min && y <= y_max && x >= x_min && x <= x_max);
}
