#include "filemanagement.h"

fileManagement::fileManagement()
{

}

void fileManagement::createFile(QString fileName)
{
    QFile mfile(fileName);
    if (!mfile.exists())
    {
        if (mfile.open(QFile::ReadWrite)) mfile.close();
        else fileErrors("Creating File Error", mfile.errorString());
    }
}

void fileManagement::writeToFile(QString fileName, QString content)
{
    QFile mfile(fileName);
    QTextStream out(&mfile);
    if (!mfile.open(QFile::ReadWrite))
    {
        fileErrors("Write To File Error", mfile.errorString());
    }
    else
    {
        out << content;
    }
    mfile.close();
}

void fileManagement::appendToFile(QString fileName, QString content)
{
    QFile mfile(fileName);
    QTextStream out(&mfile);
    if (!mfile.open(QFile::Append))
    {
        fileErrors("Append To File Error", mfile.errorString());
    }
    else
    {
        out << content;
    }
    mfile.close();
}

QStringList fileManagement::readFromFile(QString fileName)
{
    QFile mfile(fileName);
    QTextStream in(&mfile);
    QStringList myContent;
    if (!mfile.open(QFile::ReadOnly))
    {
        fileErrors("Read From File Error", mfile.errorString());
    }
    else
    {
        while (!in.atEnd())
        {
            QString line = in.readLine();
            line = line.trimmed();
            myContent.append(line);
        }
    }
    mfile.close();
    return myContent;
}

void fileManagement::fileErrors(QString title, QString errorMsg)
{
    QMessageBox msg;
    msg.setWindowTitle(title);
    msg.setText(errorMsg);
    msg.exec();
}

bool fileManagement::checkIfFileExists(QString fileName)
{
    QFile mfile(fileName);
    if(mfile.exists()) return true;
    return false;
}

QString fileManagement::readWoWPath(QString fileName)
{
    QString naam;
    QFile mfile(fileName);
    QTextStream in(&mfile);
    if (!mfile.open(QFile::ReadOnly))
    {
        fileErrors("Read From File Error", mfile.errorString());
        return "";
    }
    else
    {
        naam = in.readAll();
        mfile.close();
    }
    return naam;
}

void fileManagement::deleteFiles(QString fileName)
{
    QFile mfile(fileName);
    if(mfile.exists())
    {
        mfile.remove();
    }
    else
    {
        fileErrors("Delete File Error", mfile.errorString());
    }
}

