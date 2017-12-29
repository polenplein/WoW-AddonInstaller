#include "dbmanager.h"

dbManager::dbManager()
{

}

bool dbManager::openDB(QString dbName)
{
    db.setDatabaseName(dbName);
    if(db.open())
    {
        return true;
    }
    setError("Database Error !<b>", dbName + "</b>\n" + db.lastError().text());
    return false;
}

void dbManager::closeDB()
{
    db.close();
}

void dbManager::setError(QString windowTitle, QString errorMsg)
{
    QMessageBox msg;
    msg.setWindowTitle(windowTitle);
    msg.setText(errorMsg);
    msg.exec();
}

void dbManager::createTableAllAddons()
{
    QSqlQuery query;
    QString sql = "CREATE TABLE allAddons ("
                  "id INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE, "
                  "lurl TEXT, "
                  "lname TEXT, "
                  "indb TEXT, "
                  "ldescr TEXT)";
    if(!query.exec(sql))
    {
        setError("QSL Query Error!",  "<b>CREATE TABLE allAddons</b>\n" + query.lastError().text());
    }
}

void dbManager::createTableMyAddons()
{
    QSqlQuery query;
    QString sql = "CREATE TABLE IF NOT EXISTS myAddons ("
                  "id INTEGER PRIMARY KEY  AUTOINCREMENT  NOT NULL  UNIQUE, "
                  "installedon TEXT, "
                  "addonName TEXT, "
                  "needupdate TEXT, "
                  "version TEXT, "
                  "downurl TEXT, "
                  "deletefile TEXT, "
                  "myInstallEpoch TEXT) ";
    if(!query.exec(sql))
    {
        setError("QSL Query Error!",  "<b>CREATE TABLE myAddons</b>\n" + query.lastError().text());
    }
}
