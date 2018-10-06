#ifndef DBMANAGER_H
#define DBMANAGER_H
#include <QtSql>
#include "globals.h"
using namespace std;

class DbManager : public QThread
{
    Q_OBJECT
public:
    explicit DbManager(QObject *parent = nullptr);
    ~DbManager();
    void run();
public slots:
    void executeQuery(QListPacket,int, int);
    void selectQuery(uint,uint);
signals:
    void doUpdateGrid(QListPacket);
private:
     QSqlDatabase db;
};

#endif // DBMANAGER_H
