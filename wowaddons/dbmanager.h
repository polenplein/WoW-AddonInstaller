#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QtSql>
#include <QMessageBox>
#include <QDebug>




class dbManager
{
public:
    dbManager();

public slots:
    bool openDB(QString dbName);
    void closeDB();
    void setError(QString windowTitle, QString errorMsg);
    void createTableAllAddons();
    void createTableMyAddons();

private:
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
};

#endif // DBMANAGER_H
