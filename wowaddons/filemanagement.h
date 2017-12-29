#ifndef FILEMANAGEMENT_H
#define FILEMANAGEMENT_H
#include <QString>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDebug>




class fileManagement
{
public:
    fileManagement();

    void createFile(QString fileName);
    void writeToFile(QString fileName, QString content);
    void appendToFile(QString fileName, QString content);
    QStringList readFromFile(QString fileName);
    void fileErrors(QString title, QString errorMsg);
    bool checkIfFileExists(QString fileName);
    QString readWoWPath(QString fileName);
    void deleteFiles(QString fileName);

};

#endif // FILEMANAGEMENT_H
