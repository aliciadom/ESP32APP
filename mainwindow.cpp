#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->dateTimeFrom->setDateTime(QDateTime::currentDateTime().addSecs(-60*5));
    configureESP32Position();
    setupGridLayout();
    setupChartLayout();
    connect(ui->startButton, SIGNAL (released()), this, SLOT (realTimeButtonSlot()));
    connect(ui->buttonUpdate, SIGNAL (released()), this, SLOT (buttonUpdateSlot()));
    connect(this,SIGNAL(querySignal(uint,uint,QListESP32)), &db, SLOT(querySlot(uint,uint,QListESP32)));
    connect(&db, SIGNAL(updateGridSignal(QMapHashPacket)), this, SLOT(updateGridLayoutSlot(QMapHashPacket)));
    connect(this,SIGNAL(updateCellGridSignal(int, int, QString)),this,SLOT(updateCellGridSlot(int, int, QString)));
    connect(&timer,SIGNAL(timeout()), this, SLOT(updateRealTimeSlot()));
    connect(this,SIGNAL(oneShotSignal()), this, SLOT(updateRealTimeSlot()));
    connect(ui->spinBox, SIGNAL(valueChanged(int)),this, SLOT(setAccuracySlot()));
    connect(this,SIGNAL(updateChartLayoutSignal(QMapList)),this,SLOT(updateChartLayoutSlot(QMapList)));



    qRegisterMetaType<QPackets>("QPackets");
    qRegisterMetaType<QChartMap>("QChartMap");

    thread = new QThread();
    worker = new Worker();
    worker->moveToThread(thread);
    connect(this,SIGNAL(doWorkSignal()), worker, SLOT(doWorkSlot()));
    connect(this,SIGNAL(stopWorkerSignal()), worker, SLOT(stopWorkerSlot()));
    connect(worker, SIGNAL(resultReadySignal(QPackets, QChartMap, int)), this, SLOT(updateGridLayoutSlot2(QPackets, QChartMap, int)));


    db.start();
    thread->start();
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
           x_max = -1;
           x_min = COLUMN+1;
           y_max = -1;
           y_min = ROW+1;
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
           //line = in.readLine();
           //accuracy = stoi(line.toUtf8().constData());
           accuracy = ui->spinBox->value();
           qDebug() << accuracy;
           qDebug() << x_min;
           qDebug() << x_max;
           qDebug() << y_min;
           qDebug() << y_max;
           file.close();
           ui->nesp32label->setText(QString::number(nesp32));
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
                qw->setToolTip(s);
                qw->setPalette(empty);
            }
        }
    }
}
/*
 * Real time mode
*/
void MainWindow::realTimeButtonSlot()
{
    if(ui->startButton->text()=="AVVIA")
    {
            ui->startButton->setText("FERMA");
            ui->deviceslabel->setText(QString::number(0));
            //switch off
            ui->buttonUpdate->setEnabled(false);
            ui->dateTimeFrom->setEnabled(false);
            //ui->spinBox->setEnabled(false);
            //ui->checkBox->setEnabled(false);
            emit oneShotSignal();
            timer.start(1000*60);

    }
    else
    {
        ui->startButton->setText("AVVIA");
        timer.stop();
        emit stopWorkerSignal();
        //switch on
        ui->buttonUpdate->setEnabled(true);
        ui->dateTimeFrom->setEnabled(true);
        //ui->spinBox->setEnabled(true);
        //ui->checkBox->setEnabled(true);

    }

}
void MainWindow::buttonUpdateSlot()
{
    if(ui->buttonUpdate->text()=="AGGIORNA")
    {
            ui->buttonUpdate->setText("ANNULLA ANALISI");
            QDateTime from = ui->dateTimeFrom->dateTime();
            uint unixtimefrom = from.toTime_t();
            uint unixtimeto = unixtimefrom+(60*5);
            ui->deviceslabel->setText(QString::number(0));
            distinctdevices.clear();
            emit querySignal(unixtimefrom,unixtimeto,esp32devices.values());

    }
    else
    {
        ui->buttonUpdate->setText("AGGIORNA");
        emit stopWorkerSignal();

    }

}
void MainWindow::updateGridLayoutSlot(QMapHashPacket hashmap)
{
    QMap<uint,QList<string>> chartmap;
    qDebug() << "updateGridLayoutSlot()..";
    distinctdevices.clear();

    worker->SetheckBoxIsChecked(ui->checkBox->isChecked());
    worker->SetQMapHashPacket(hashmap);
    worker->setAccuracy(accuracy);
    worker->setUtil(nesp32,x_min,x_max,y_min,y_max);
    worker->setEsp32Devices(esp32devices);

    if(!hashmap.isEmpty())
    {
        //ui->buttonUpdate->setEnabled(false);
        emit doWorkSignal();
    }
    else
    {
            ui->buttonUpdate->setText("AGGIORNA");
    }

    qDebug() << "updateGridLayoutSlot() finished";
}
void MainWindow::updateGridLayoutSlot2(QPackets packets , QChartMap chartmap, int ndevices)
{
    clearGridLayout();
    for(QPair<Packet,QPoint> qp : packets)
    {
        QString s;
        Packet p = qp.first;
        QPoint point = qp.second;
        string packethash = p.getHash();
        string mac = p.getMAC();
        uint unixtime = p.getTimestamp();
        QDateTime time;
        time.setTime_t(unixtime);
        s.append("{\n\tMAC: ").append(QString::fromUtf8(p.getMAC().c_str())).append("\n\t:").append(time.toString()).append("\n}\n");
        qDebug() << QString::fromStdString(p.toString());
        emit updateCellGridSignal(point.ry(),point.rx(),s);
    }
    emit updateChartLayoutSignal(chartmap);
    if((ui->buttonUpdate->text()!="AGGIORNA"))
        ui->buttonUpdate->setText("AGGIORNA");
    ui->deviceslabel->setText(QString::number(ndevices));

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
void MainWindow::setAccuracySlot()
{
    accuracy = ui->spinBox->value();
}

void MainWindow::updateRealTimeSlot()
{
    QDateTime to = QDateTime::currentDateTime();
    uint unixtimeto = to.toTime_t();
    uint unixtimefrom = unixtimeto-(60*5);
    ui->deviceslabel->setText(QString::number(0));
    distinctdevices.clear();
    emit querySignal(unixtimefrom,unixtimeto,esp32devices.values());
}
/*
 * chart
*/
void MainWindow::setupChartLayout()
{
    QDateTime from = ui->dateTimeFrom->dateTime();
    QDateTime to = from.addSecs(60*5);
    QStringList categories = {from.toString("hh:mm:ss"), to.toString("hh:mm:ss")};
    QBarSet *high = new QBarSet("Numero Dispositivi");
    axisX= new QBarCategoryAxis();
    chart = new QChart();
    series = new QStackedBarSeries();
    *high << 0 << 0;
    //Y
    axisY = new QValueAxis();
    axisY->setTickCount(2);
    axisY->setLabelFormat("%i");
    axisY->setRange(0,1);
    //X
    QFont labelsFont;
    labelsFont.setPixelSize(10);
    axisX->setTitleText(to.date().toString());
    axisX->append(categories);
    axisX->setLabelsFont(labelsFont);
    //series
    series->append(high);

    /*chart*/
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setAxisX(axisX,series);
    chart->setAxisY(axisY,series);

    chartView = new QChartView(chart);
    ui->verticalLayout->addWidget(chartView);
}
void MainWindow::updateChartLayoutSlot(QMapList map)
{
    qDebug() << "updateChartLayoutSlot().. called";
    if(!map.empty())
    {
        qDebug() << "updateChartLayoutSlot() some values..";
        int maxvalue = 1;
        series->clear();
        axisX->clear();
        QStringList categories;
        QBarSet *high = new QBarSet("Numero Dispositivi");
        high->append(0); //
        categories.push_back(ui->dateTimeFrom->time().toString("hh:mm:ss"));
        for(uint key : map.keys())
        {
            int value = map[key].size();
            if(value > maxvalue) maxvalue = value;
            high->append(value);
            categories.push_back(QDateTime::fromTime_t(key).toString("hh:mm:ss"));
        }
        high->append(0); //
        categories.push_back(ui->dateTimeFrom->time().addSecs(60*5).toString("hh:mm:ss"));
        axisX->append(categories);
        axisX->setTitleText(ui->dateTimeFrom->date().toString());
        series->append(high);
        axisY->setRange(0,maxvalue);
        axisY->setTickCount(1+maxvalue);
    }
    else
    {
        qDebug() << "updateChartLayoutSlot() no values..";
        series->clear();
        axisX->clear();
        QStringList categories;
        QBarSet *high = new QBarSet("NumeroDispositivi");
        high->append(0); //
        categories.push_back(ui->dateTimeFrom->time().toString("hh:mm:ss"));
        high->append(0); //
        categories.push_back(ui->dateTimeFrom->time().addSecs(60*5).toString("hh:mm:ss"));
        axisX->append(categories);
        axisX->setTitleText(ui->dateTimeFrom->date().toString());
        series->append(high);
        axisY->setRange(0,1);
        axisY->setTickCount(2);
    }
}


