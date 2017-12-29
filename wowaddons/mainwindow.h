#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "filemanagement.h"
#include "dbmanager.h"
#include <QStandardPaths>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QResource>
#include <QtNetwork>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QSqlTableModel>





namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QString workdir = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString(), QStandardPaths::LocateDirectory) + "MyWowAddons/";
    QString InstalledAddons = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QString(), QStandardPaths::LocateDirectory) + "MyWowAddons/InstalledAddons/";
    QString homeDir = QStandardPaths::locate(QStandardPaths::HomeLocation, QString(), QStandardPaths::LocateDirectory);
    QString htlmPage = workdir + "htlmPage.txt";
    QString downPage = workdir + "downPage.txt";
    QString dataBaseName = workdir + "MyAddons.sqlite";
    QString wowSettings = workdir + "WowSettings.txt";
    QString fieldId, addonId, downUrl, addonName,downloadDatum, zipF, wowPath,updateText;
    int maxPages, addonTeller;


public slots:
    bool doDownload(QString urlPath);
    QString fetchHtmlPage(QString url);
    void verwerkHtlm(QStringList content);
    void getMaxPages(QStringList content);
    QString replaceHtlmEntities(QString string);
    QString makeFilesDbReady(QString fileName);

private slots:
    void on_actionExit_triggered();

    void on_actionUpdate_triggered();

    void on_actionWoW_Folder_triggered();

    void on_actionMy_Addons_triggered();

    void on_actionInstall_Addons_triggered();

    void on_tableWidgetInstallAddons_cellClicked(int row, int column);

    void on_toolButtonInstallAddon_clicked();

    void on_lineEditSearchNewAddons_textChanged(const QString &arg1);

    void on_toolButtonMyAddonsDelete_clicked();

    void on_toolButtonMyAddonsUpdate_clicked();

    void on_tableWidgetMyAddons_cellClicked(int row, int column);

    void on_actionHelp_triggered();

private:
    Ui::MainWindow *ui;
    fileManagement myFiles;
    QNetworkAccessManager * manager = new QNetworkAccessManager(this);
    dbManager db;
    QSqlTableModel * model = new QSqlTableModel(this);
};

#endif // MAINWINDOW_H
