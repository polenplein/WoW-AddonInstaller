#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMap>
#include <QFileDialog>
#include <QFileInfo>
#include <QString>
#include <QDebug>
#include <QPushButton>
#include <QStandardItem>
#include <QTableWidget>
#include <QStyle>
#include <QDesktopWidget>
#include <QDateTime>
#include <JlCompress.h>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setGeometry(
        QStyle::alignedRect(
            Qt::LeftToRight,
            Qt::AlignCenter,
            this->size(),
            qApp->desktop()->availableGeometry()
        )
    );
    ui->stackedWidget->setCurrentIndex(0);
    QDir mdir(workdir);
    if (!mdir.exists()) mdir.mkpath(workdir);
    QDir idir(InstalledAddons);
    if (!idir.exists()) idir.mkpath(InstalledAddons);
    myFiles.createFile(htlmPage);
    myFiles.createFile(dataBaseName);
    myFiles.createFile(InstalledAddons);
    bool ok = myFiles.checkIfFileExists(wowSettings);
    if (!ok)
    {
        myFiles.checkIfFileExists(wowSettings);
        QFileDialog myDialog;
        myDialog.setHidden(true);

        QString pnaam = myDialog.getExistingDirectory(this,"Choose your World of Warcraft Installation Folder",homeDir,myDialog.ShowDirsOnly);
        if (!pnaam.contains("Interface"))
        {
            pnaam = pnaam + "/Interface/AddOns";
        }
        if (!pnaam.contains("AddOns"))
        {
            pnaam = pnaam + "/AddOns";
        }
        pnaam = pnaam + "/";
        myFiles.writeToFile(wowSettings,pnaam);
    }
    db.openDB(dataBaseName);
    db.createTableMyAddons();
}

MainWindow::~MainWindow()
{
    db.closeDB();
    delete ui;
}


void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionUpdate_triggered()
{
    ui->textBrowserUpdateDB->setOpenExternalLinks(true);
    QString upText = "Updating the database is time consuming, but you don't need to do that on a daily base.<br><br>"
            "If you hear about a new addon, and it's not in the database, then it's time to update the database.<br><br>"
            "I need to go out to the Curse webpage  to fetch the information I need. "
            "Reading the html pages and extract the info that I need takes time. "
            "Sorry for that, but it's the only way to get it.<br><br>"
            "Get a cup of coffee or go for a smoke :) On my computer it takes about 6 minutes.<br><br>"
            "If for some reason the update is interrupted, you need to re-update :(<br>";
    ui->textBrowserUpdateDB->setText("<b>" + upText + "</b>");
    ui->textBrowserUpdateDB->append("<b><font color='green'>Database Location: " + dataBaseName + ".</b></font><br>");
    ui->textBrowserUpdateDB->append("<b><font color='red'>Donations via PayPal to further maintain this application is highly appreciated.<b></font><br>");
    ui->textBrowserUpdateDB->append("<a href='https://www.paypal.me/SergeTerryn'><b>Serge Terryn, BE Roeselare</b></a>");
    //https://www.paypal.com/paypalme/SergeTerryn
    if (!db.openDB(dataBaseName))
    {
        qFatal("Can't open the Database !");
    }
    QString sql = "DROP TABLE IF EXISTS allAddons";
    QSqlQuery query;
    if (!query.exec(sql))
    {
        db.setError("SQL Query Error: 96", query.lastError().text());
    }
    db.createTableAllAddons();

    addonTeller = 0;
    ui->stackedWidget->setCurrentIndex(1);

    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
    QString url = "https://www.curseforge.com/wow/addons?filter-game-version=&filter-sort=updated";
    QString htmlContent = fetchHtmlPage(url);
    myFiles.writeToFile(htlmPage, htmlContent);
    QStringList wPage = myFiles.readFromFile(htlmPage);
    getMaxPages(wPage);
    verwerkHtlm(wPage);
    ui->progressBar->setMaximum(maxPages);
    for(int i = 2; i <= maxPages; i++)
    {
        ui->progressBar->setValue(i);
        url = "https://www.curseforge.com/wow/addons?filter-game-version=&filter-sort=updated&page=" + QString::number(i);
        QString htmlContent = fetchHtmlPage(url);
        myFiles.writeToFile(htlmPage, htmlContent);
        wPage = myFiles.readFromFile(htlmPage);
        verwerkHtlm(wPage);
    }
}

void MainWindow::getMaxPages(QStringList content)
{
    QStringList str;
    foreach (QString line, content)
    {
        if (line.contains("b-pagination-item s-active active"))
        {
            line = line.replace("\"","");
            line = line.replace("class=dots>&hellip;</li><li class=b-pagination-item><a href=/wow/addons?filter-sort=updated&page=",",");
            line = line.replace(" class=b-pagination-item>",",");
            line = line.replace("</a></li><li,<a href=/wow/addons?filter-sort=updated&amp;page=",",");
            line = line.replace(" rel=next data-next-page>Next</a></li>","");
            str = line.split(",");
            QString MaxPages = str[11];
            maxPages = MaxPages.toInt();
        }
    }
}

QString MainWindow::replaceHtlmEntities(QString string)
{
    if (string.contains("&amp;"))
    {
        string = string.replace("&amp;", "&");
    }
    if (string.contains("&#x27;"))
    {
        string = string.replace("&#x27;", "''");
    }
    if (string.contains("&quot;"))
    {
        string = string.replace("&quot;", "");
    }
    return string;
}

QString MainWindow::makeFilesDbReady(QString fileName)
{
    if(fileName.contains("'")) fileName = fileName.replace("'","''");
    if(fileName.contains(";")) fileName = fileName.replace("'","-");
    return fileName;
}


void MainWindow::verwerkHtlm(QStringList content)
{

    QString lurl,lname, ldescr;
    QStringList str;
    foreach (QString line, content)
    {
        if (line.contains("<p title=\""))
        {
            line = line.replace("</p>","");
            str = line.split(">");
            ldescr = str[1];
            ldescr = replaceHtlmEntities(ldescr);
        }
        if(line.contains("button--download button-"))
        {
            line = line.replace("\"","");
            //<a class=button button--download button--cta button--icon align-center [/wow/addons/wowhow/download[ data-action-value='{ProjectID: 102774, [&quot;WowHow&quot;}'>

            line = line.replace("href=","[");
            line = line.replace(" data-action=download-project","[");
            line = line.replace("ProjectName: ","[");
            line = line.replace("}'>","");
            str = line.split("[");
            lurl = str[1];
            lurl = "https://www.curseforge.com" + lurl;
            lname = str[3];
            lname = replaceHtlmEntities(lname);
            lname = makeFilesDbReady(lname);
            QString sql = "INSERT INTO allAddons (lurl,lname,indb,ldescr) VALUES ('"+lurl+"','"+lname+"', 'no','"+ldescr+"')";
            QSqlQuery qry;
            if(!qry.exec(sql))
            {
                db.setError("SQL Query Error: 198", qry.lastError().text());
            }
            addonTeller++;
            ui->labelAddonsFound->setText("<b>Addons Found: " + QString::number(addonTeller) + "</b>");
        }
    }
}

bool MainWindow::doDownload(QString urlPath)
{
    QString version,znaam;
    QStringList vv;
    //qDebug() << "Download url: " << urlPath;
    QString dPage = fetchHtmlPage(urlPath);
    myFiles.writeToFile(downPage, dPage);
    QStringList myPage = myFiles.readFromFile(downPage);
    foreach (QString o, myPage)
    {
        if(o.contains("<meta property=\"og:title\""))
        {
            o = o.replace("<meta property=\"og:title\" content=\"","");
            znaam = o.replace("\" />","");
        }
        if(o.contains("Game Version"))
        {
            o = o.replace("</span>","");
            vv = o.split(":");
            o = vv[1];
            version = o.trimmed();
        }
        if (o.contains("start automatically"))
        {
            //qDebug() << "start automatically: " << o;
            o = o.replace("\"","");
            o = o.replace("<p class=>if your download doesn't start automatically, click <a class=download__link href=/","");
            o = o.replace(">here</a></p>","");
            downUrl = "https://www.curseforge.com/" + o;
        }
    }

    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setUrl(downUrl);
    QNetworkReply* reply = manager->get(request);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();
    QStringList zipInhoud;
    QString listFile;

    if(reply->error())
        {
            qDebug() << "ERROR!";
            qDebug() << reply->errorString();
        }
        else
        {
            wowPath = myFiles.readWoWPath(wowSettings);
            zipF = znaam + ".zip";
            //qDebug() << "Name of zipFile: " << zipF;
            //qDebug() << "Name of wowPath: " << wowPath;
            QFile file(zipF);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(reply->readAll());
                file.close();
            }
            else
            {
                qDebug() << "Could not open file for writing: " << zipF << file.errorString();
            }
            JlCompress zFile;
            zipInhoud = zFile.getFileList(zipF);
            listFile= InstalledAddons + znaam + "Files.dat";

            myFiles.createFile(listFile);
            QString content;
            foreach(QString h, zipInhoud)
            {
                QStringList line = h.split("/");
                QString s = line[0];
                if(!content.contains(s))
                {
                    content += wowPath + s + "\n";
                }
            }
            myFiles.writeToFile(listFile,content);
            //qDebug() << "zipInhoud: " << zipInhoud;
            JlCompress::extractDir(zipF, wowPath);
            QDate myDate = QDate::currentDate();
            QString myEpochInstall = QString::number(QDateTime::currentSecsSinceEpoch());
            //qDebug() << "myEpoch: " << myEpochInstall;
            int dag = myDate.day();
            QString dags = QString::number(dag);
            if(dags.length() < 2) dags = "0" + dags;
            int maand = myDate.month();
            QString maands = QString::number(maand);
            if(maands.length() < 2) maands = "0" + maands;
            int jaar = myDate.year();
            downloadDatum = dags + maands + QString::number(jaar);

            QString dbAddon = makeFilesDbReady(addonName);

            if(updateText == "install")
            {
                listFile = makeFilesDbReady(listFile); //listFile.replace("'", "''");
                QSqlQuery qry;
                if(!qry.exec("INSERT INTO myAddons (installedon,addonName,downurl,needupdate,version,deletefile,myInstallEpoch) values ("
                         "'"+downloadDatum+"',"
                         "'"+dbAddon+"',"
                         "'"+downUrl+"',"
                         "'Up to date',"
                         "'"+version+"',"
                         "'"+listFile+"',"
                         "'"+myEpochInstall+"')")) db.setError("SQL Query Error: 307", qry.lastError().text());
                QMessageBox msg;
                msg.setWindowTitle("Installing Addons");
                msg.setText("<b>The addon " + addonName + " is installed.</b>");
                msg.exec();

            }

            if(updateText=="yes")
            {
                QSqlQuery qry;
                if(!qry.exec("UPDATE myAddons set needupdate='Up To Date', "
                             "myInstallEpoch='"+myEpochInstall+"', "
                             "installedon='"+downloadDatum+"' WHERE id='"+addonId+"'")) db.setError("SQL Query Error: 320", qry.lastError().text());
                return true;
            }
        }
    return true;
}

QString MainWindow::fetchHtmlPage(QString url)
{
    //qDebug() << url;
    QNetworkRequest request;
    request.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    request.setUrl(url);
    QNetworkReply* reply = manager->get(request);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)), &loop, SLOT(quit()));
    loop.exec();
    QByteArray bts = reply->readAll();
    QString str(bts);
    return str;
}


void MainWindow::on_actionWoW_Folder_triggered()
{
    QFileDialog myDialog;

    QString pnaam = myDialog.getExistingDirectory(this,"Choose your World of Warcraft Installation Folder",homeDir,myDialog.ShowDirsOnly);
    if (!pnaam.contains("Interface"))
    {
        pnaam = pnaam + "/Interface/AddOns";
    }
    if (!pnaam.contains("AddOns"))
    {
        pnaam = pnaam + "/AddOns";
    }
    pnaam = pnaam + "/";
    myFiles.writeToFile(wowSettings,pnaam);
}

void MainWindow::on_actionMy_Addons_triggered()
{
    ui->stackedWidget->setCurrentIndex(2);
    QString RedStyle = "QTableWidget::item {"
        "background-color: red;"
        "}";
    QString WhiteStyle = "QTableWidget::item {"
        "background-color: white;"
        "}";
        //tableWidget->setStyleSheet(_CustomStyle);

    QSqlQuery query;
    if (!query.exec("SELECT count(id) as num_rows FROM myAddons")) db.setError("SQL Query Error: 373", query.lastError().text());
    query.first();
    int num_rows = query.value(0).toInt();
    int tel = 0;
    ui->progressBarMyAddons->setMinimum(0);
    ui->progressBarMyAddons->setMaximum(num_rows);
    ui->progressBarMyAddons->setValue(tel);


    if(!query.exec("SELECT id, downurl, myInstallEpoch from myAddons")) db.setError("SQL Query Error: 382", query.lastError().text());
    while(query.next())
    {
        QString dUrl = query.value(1).toString();
        dUrl = dUrl.replace("download/", "download[");
        QStringList dd = dUrl.split("[");
        dUrl = dd[0];
        QString dPage = fetchHtmlPage(dUrl);
        myFiles.writeToFile(downPage, dPage);

        QStringList myPage = myFiles.readFromFile(downPage);
        foreach(QString i, myPage)
        {
            QString epo = "";
            if(i.contains("Last Updated: <abbr")){
                i = i.trimmed();
                i.replace("\"","");
                i = i.replace("data-epoch=", "[");
                QStringList dd = i.split("[");
                i = dd[1];
                for(int x = 0; x < 10; ++x)
                {
                    epo += i.at(x);
                }
                //qDebug() << epo;
                if (epo > query.value(2).toString())
                {
                    //qDebug() << "update voor record: " + query.value(0).toString();
                    QSqlQuery qry;
                    if(!qry.exec("UPDATE myAddons set needupdate='Update Available' where id='"+query.value(0).toString()+"'")) db.setError("SQL Query Error: 411", query.lastError().text());
                }
            }
        }
        tel++;

        ui->progressBarMyAddons->setValue(tel);
    }
    if(!query.exec("SELECT count(id) as rc FROM myAddons")) db.setError("SQL Query Error: 419", query.lastError().text());
    query.first();
    int nr = query.value(0).toInt();

    ui->tableWidgetMyAddons->setRowCount(nr);
    ui->tableWidgetMyAddons->setColumnCount(5);
    QStringList myView;
    myView << "id" << "Addon Name" << "Installed On" << "Game Version" << "Update";
    ui->tableWidgetMyAddons->setHorizontalHeaderLabels(myView);
    if((!query.exec("SELECT * FROM myAddons ORDER BY needupdate DESC"))) db.setError("SQL Query Error: 428", query.lastError().text());
    //qDebug() << query.lastQuery();
    int rc = 0;
    while(query.next())
    {
        QString datum = query.value(1).toString();
        datum.insert(2,"-");
        datum.insert(5,"-");
        ui->tableWidgetMyAddons->setColumnHidden(0, true);
        ui->tableWidgetMyAddons->setItem(rc,0, new QTableWidgetItem(query.value(0).toString()));
        ui->tableWidgetMyAddons->setItem(rc,1, new QTableWidgetItem(query.value(2).toString()));
        ui->tableWidgetMyAddons->setItem(rc,2, new QTableWidgetItem(datum));
        ui->tableWidgetMyAddons->setItem(rc,3, new QTableWidgetItem(query.value(4).toString()));

        ui->tableWidgetMyAddons->setItem(rc,4, new QTableWidgetItem(query.value(3).toString()));

        rc++;
    }
    ui->tableWidgetMyAddons->setAlternatingRowColors(true);
    ui->tableWidgetMyAddons->resizeColumnToContents(true);

}

void MainWindow::on_actionInstall_Addons_triggered()
{

    ui->stackedWidget->setCurrentIndex(3);

    QSqlQuery query, qry;
    QStringList myView;
    int num_rows;
    if (!query.exec("SELECT count(id) as num_rows FROM allAddons where indb='no'")) db.setError("SQL Query Error: 458", query.lastError().text());
    query.first();
    num_rows = query.value(0).toInt();
    ui->tableWidgetInstallAddons->setRowCount(num_rows);
    ui->tableWidgetInstallAddons->setColumnCount(2);

    myView << "id" << "Addon Name";
    ui->tableWidgetInstallAddons->setHorizontalHeaderLabels(myView);
    QString sql = "SELECT id,lname,indb FROM allAddons where indb='no'";
    if (!qry.exec(sql)) db.setError("SQL Query Error: 474", qry.lastError().text());
    int tel=0;
    while (qry.next())
    {
        QString v0 = qry.value(0).toString();
        QString v1  = qry.value(1).toString();
        ui->tableWidgetInstallAddons->setColumnHidden(0, true);
        ui->tableWidgetInstallAddons->setItem(tel,0, new QTableWidgetItem(v0));
        ui->tableWidgetInstallAddons->setItem(tel,1, new QTableWidgetItem(v1));
        ++tel;
    }
    ui->tableWidgetInstallAddons->setAlternatingRowColors(true);
    ui->tableWidgetInstallAddons->resizeColumnToContents(true);
}


void MainWindow::on_tableWidgetInstallAddons_cellClicked(int row, int column)
{

    QString id = ui->tableWidgetInstallAddons->item(row,0)->text();
    if (id != "")
    {
        addonId = id;
        QString txt = ui->tableWidgetInstallAddons->item(row,column)->text();
        txt = makeFilesDbReady(txt);
        QSqlQuery q;
        if(!q.exec("SELECT ldescr FROM allAddons where lname='"+txt+"'")) db.setError("SQL Query Error: 491", q.lastError().text());
        q.first();
        QString descr = "<b><font color='red'>" + q.value(0).toString() + "</font></b>";
        ui->textBrowserDescr->setText(descr);
    }
}

void MainWindow::on_toolButtonInstallAddon_clicked()
{
    updateText="install";
    ui->lineEditSearchNewAddons->setText("");
    QSqlQuery qry;
    if (!qry.exec("SELECT id, lurl, lname FROM allAddons where id='"+addonId+"'")) db.setError("SQL Query Error: 502", qry.lastError().text());
    qry.first();
    addonId = qry.value(0).toString();
    downUrl = qry.value(1).toString();
    addonName = qry.value(2).toString();
    if(!qry.exec("UPDATE allAddons set indb='yes' where id='"+addonId+"'")) db.setError("SQL Query Error: 507", qry.lastError().text());

    doDownload(downUrl);

}

void MainWindow::on_lineEditSearchNewAddons_textChanged(const QString &arg1)
{
    QString searc = arg1;
    QSqlQuery q;
    int num_rows;
    QString sql = "SELECT count(id) as num_rows FROM allAddons where indb='no'";
    if (!q.exec(sql)) db.setError("SQL Query Error: 519", q.lastError().text());
    q.first();
    num_rows = q.value(0).toInt();
    ui->tableWidgetInstallAddons->setRowCount(num_rows);
    ui->tableWidgetInstallAddons->setColumnCount(2);

    QStringList myView;
    myView << "id" << "Addon Name";
    ui->tableWidgetInstallAddons->setHorizontalHeaderLabels(myView);

    sql = "SELECT id,lname FROM allAddons WHERE lname LIKE '"+searc+"%' AND indb='no'";
    if(!q.exec(sql)) db.setError("SQL Query Error ! (530)", q.lastError().text());
    int r = 0;
    while(q.next())
    {
        QString v0 = q.value(0).toString();
        QString v1 = q.value(1).toString();
        ui->tableWidgetInstallAddons->setColumnHidden(0, true);
        ui->tableWidgetInstallAddons->setItem(r,0, new QTableWidgetItem(v0));
        ui->tableWidgetInstallAddons->setItem(r,1, new QTableWidgetItem(v1));
        r++;
    }
    ui->tableWidgetInstallAddons->setAlternatingRowColors(true);
}

QString deleteId, deleteTxt, deleteFile;
void MainWindow::on_toolButtonMyAddonsDelete_clicked()
{
    QStringList fileContent = myFiles.readFromFile(deleteFile);
    foreach (QString s, fileContent) {
        QDir mdir(s);
        mdir.removeRecursively();
        //qDebug() << "dir: " << s;
    }
    QSqlQuery query;
    QString deltxt = makeFilesDbReady(deleteTxt);
    QString sql = "UPDATE allAddons set indb='no' where lname='"+deltxt+"'";
    if(!query.exec(sql)) db.setError("SQL Query Error 556", query.lastError().text());
    sql = "DELETE FROM myAddons where id='"+deleteId+"'";
    if(!query.exec(sql)) db.setError("SQL Query Error: 558", query.lastError().text());
    myFiles.deleteFiles(deleteFile);
    on_actionMy_Addons_triggered();
}

void MainWindow::on_toolButtonMyAddonsUpdate_clicked()
{
    updateText = "yes";
    QString msg = "";
    QSqlQuery query;
    if (!query.exec("SELECT id, downurl, needupdate, addonName FROM myAddons WHERE needupdate='Update Available'"))db.setError("SQL Query Error: 568", query.lastError().text());
    while (query.next())
    {
        addonName = query.value(3).toString();
        addonId = query.value(0).toString();
        if(doDownload(query.value(1).toString()))
        {
            msg += query.value(3).toString() + " Updated !\n";
        }
    }
    db.setError("Updating Addons",msg);
    on_actionMy_Addons_triggered();
}

void MainWindow::on_tableWidgetMyAddons_cellClicked(int row, int column)
{
    QSqlQuery query;
    deleteId = ui->tableWidgetMyAddons->item(row,0)->text();
    deleteTxt= ui->tableWidgetMyAddons->item(row,column)->text();
    QString sql = "SELECT deletefile,addonName FROM myAddons where id='"+deleteId+"'";
    if(!query.exec(sql)) db.setError("SQL Query Error: 588", query.lastError().text());
    query.first();
    deleteFile = query.value(0).toString();
    deleteTxt = query.value(1).toString();

}

void MainWindow::on_actionHelp_triggered()
{
    ui->stackedWidget->setCurrentIndex(4);
    QString content;
    QResource info(":help.html");
    QFile infoFile(info.absoluteFilePath());
    if (infoFile.open(QFile::ReadOnly))
    {
        QTextStream in(&infoFile);
        content = in.readAll();
        infoFile.close();
    }
    ui->textBrowser->setText(content);

}
