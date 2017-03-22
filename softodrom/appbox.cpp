#include <QSettings>
#include <QFileIconProvider>
#include <QDesktopServices>
#include <QUrl>
#include <QMessageBox>
#include "appbox.h"
#include "ui_appbox.h"
#include "softodromglobal.h"

void appInfo::clear()
{
    iconString.clear();
    name.clear();
    dir.clear();
    description.clear();
    fullDescription.clear();
    isChecked = false;
    isAvir = false;
    mkTmp = false;
    instVercmd.clear();
    ver = SdVersion();
    instVer = SdVersion();
    url.clear();
    license.clear();
    key.clear();
    kits.clear();
    depends.clear();
    addons.clear();
    commands.clear();
    conflicts.clear();
    killTimer = 6400;
}

appBox::appBox(QWidget *parent) : QWidget(parent), ui(new Ui::appBox)
{
    ui->setupUi(this);
    state = wait;
    movie = new QMovie(":/loading.gif");
    if (movie->loopCount() != -1)
        connect(movie,SIGNAL(finished()),movie,SLOT(start()));
    pGreen.setBrush(QPalette::Active, QPalette::WindowText,QBrush(QColor(0, 128, 0, 255)));
    pOrange.setBrush(QPalette::Active, QPalette::WindowText,QBrush(QColor(255, 128, 0, 255)));
    pRed.setBrush(QPalette::Active, QPalette::WindowText,QBrush(QColor(255, 0, 0, 255)));
    pGray.setBrush(QPalette::Active, QPalette::WindowText,QBrush(QColor(128, 128, 128, 255)));
    ui->checkBox->setIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion));
    ui->checkBox->setIconSize(QSize(32,32));
    ui->installerEdit->hide();
    ui->toolButton->hide();
    ui->progressBar->hide();
    ui->extInfo->hide();
    AppSettings->beginGroup("View");
    this->setFixedHeight(80);
    this->setFixedWidth(AppSettings->value("AppBoxWidth").toInt());
    AppSettings->endGroup();
    ui->progressBar->setMaximum(0);
    ui->progressBar->setMinimum(0);
    ui->progressBar->setValue(0);
    connect(ui->checkBox,SIGNAL(clicked(bool)),this,SLOT(setChecked(bool)));
}

appBox::~appBox()
{
    delete ui;
}

void appBox::setInfo(appInfo i)
{
    semaphore++;
    this->info = i;
    info.dir.replace("/","\\");
    QString tmp;
    if (info.name.size()) ui->checkBox->setText(info.name);
    QFileIconProvider provider;
    QFileInfo *iconFile = new QFileInfo(info.iconString);
    if (iconFile->exists()) ui->checkBox->setIcon(provider.icon(*iconFile));
    if (info.description.size()) ui->description->setText(info.description);
    if (info.kits.size())
    {
        foreach(QString t,info.kits)
        {
            if (t == "avir")
            {
                info.isAvir = true;
                ui->NOWbtn->hide();
                if (info.kits.size() != 1)
                {
                    SDDebugMessage("appBox::setInfo(appInfo)",
                                   QString::fromUtf8("Запрещено включать антивирус в другие группы программ!\n"
                                                     "Ошибка при разборе %1\\info.txt").arg(i.dir),true,iconerror);
                    tmp = "avirss";
                    info.kits.clear();
                    info.kits.append("avir");
                    break;
                }
            }
            tmp += t;
            tmp += ", ";
        }
        tmp.resize(tmp.size()-2);
        ui->kits->setText(SDtranslateKit(tmp));
    }
    else
    {
        info.kits.append("nogroup");
        ui->kits->setText(SDtranslateKit("nogroup"));
    }
    clearEmptyCommands();
    if (info.commands.size())
    {
        if (info.commands.size() == 1)
        {
//            if(!info.commands.at(0).compare("auto.bat",Qt::CaseInsensitive)||!info.commands.at(0).compare("auto.cmd",Qt::CaseInsensitive))
//                info.commands[0] = info.dir+QDir::separator()+info.commands.at(0);
            ui->installerEdit->setText(info.commands.at(0));
            ui->scriptLabel->hide();
            ui->installerEdit->show();
            ui->toolButton->show();
        }else
        {
            tmp.clear();
            foreach(QString t,info.commands)
            {
                tmp += t;
                tmp += "\n";
            }
            tmp.resize(tmp.size()-1);
            ui->scriptLabel->setToolTip(tmp);
            ui->scriptLabel->show();
            ui->installerEdit->hide();
            ui->toolButton->hide();
        }
    }
    ui->checkBox->setToolTip(info.fullDescription+"\n"+info.dir);
    if (info.dir.size() && info.commands.size() && info.name.size()) state = ready;
    if (info.ver.isValid()) ui->version->setText(info.ver.toString());
    if (info.instVer.isValid())
    {
        ui->installedVer->setText(info.instVer.toString());
        if (info.ver == info.instVer)
        {
           stopInstall(normal);
           if (info.isAvir) emit avirChecked(this);
        }
        if (info.ver < info.instVer)
        {
           stopInstall(legacy);
        }
    }
    else
    {
        ui->installedVer->setText(QString::fromUtf8("Не найдено"));
    }
    if (info.url.size())
    {
        if(!QUrl(info.url).isRelative())
        {
            tmp.clear();
            tmp += "<a href=\"";
            tmp += info.url;
            tmp += "\">";
            tmp += info.url;
            tmp += "</a>";
            ui->url->setText(tmp);
            connect(ui->url,SIGNAL(linkActivated(QString)),this,SLOT(openUrl(QString)));
        }
    }
    if (info.key.size())
    {
        ui->keyEdit->setText(info.key);
        ui->keyEdit->setEnabled(true);
    }
    if (info.license.size())
    {
        if(!QUrl(info.license).isRelative())
        {
            tmp.clear();
            tmp += "<a href=\"";
            tmp += info.license;
            tmp += "\">";
            tmp += info.license;
            tmp += "</a>";
            ui->licenseLabel->setText(tmp);
            connect(ui->licenseLabel,SIGNAL(linkActivated(QString)),this,SLOT(openUrl(QString)));
        }
        else
        {
            QFileInfo licenseFile(info.license);
            if (!licenseFile.exists())
            {
                licenseFile.setFile(info.dir+QDir::separator()+info.license);
            }
            if (!licenseFile.exists())
            {
                licenseFile.setFile(ExpandEnvironmentString(info.license));
            }
            if (!licenseFile.exists())
            {
                licenseFile.setFile(info.dir+QDir::separator()+ExpandEnvironmentString(info.license));
            }
            if (licenseFile.exists())
            {
                tmp.clear();
                tmp += "<a href=\"";
                tmp += licenseFile.absoluteFilePath();
                tmp += "\">";
                tmp += info.license;
                tmp += "</a>";
                ui->licenseLabel->setText(tmp);
                connect(ui->licenseLabel,SIGNAL(linkActivated(QString)),this,SLOT(openUrl(QString)));
            }else
                ui->licenseLabel->setText(info.license);
        }
    }
    semaphore--;
}

void appBox::startWait()
{
    state = wait;
    ui->description->setPalette(pDefault);
    ui->checkBox->setPalette(pDefault);
    ui->extInfo->hide();
    ui->NOWbtn->hide();
    ui->pushButton->setText(QString::fromUtf8("Подробнее"));
    setFixedHeight(80);
    ui->checkBox->setDisabled(true);
    ui->description->setText(info.description);
    ui->pushButton->setDisabled(1);
}

void appBox::startInstall()
{
    state = setup;
    connect(movie,SIGNAL(frameChanged(int)),this,SLOT(updateMovie()));
    movie->start();
    ui->progressBar->show();
    ui->pushButton->hide();
    ui->pushButton->setEnabled(true);
}

void appBox::stopInstall(STATE st)
{
    state = st;
    ui->pushButton->show();
    ui->progressBar->hide();
    ui->pushButton->setEnabled(1);
    ui->checkBox->setEnabled(1);
    disconnect(movie,SIGNAL(frameChanged(int)),this,SLOT(updateMovie()));
    QFileIconProvider provider;
    QFileInfo *iconFile = new QFileInfo(info.iconString);
    switch(state)
    {
    case normal:
        ui->checkBox->setChecked(info.isChecked = false);
        ui->checkBox->setIcon(QIcon(":/ok.png"));
        ui->checkBox->setPalette(pGreen);
        ui->NOWbtn->hide();
        ui->description->setText(QString::fromUtf8("Установлено"));
        ui->description->setPalette(pGreen);
        break;
    case error:
        ui->checkBox->setIcon(QIcon(":/error.png"));
        ui->checkBox->setPalette(pRed);
        ui->NOWbtn->show();
        ui->description->setText(info.description);
        ui->description->setPalette(pRed);
        break;
    case warning:
        ui->checkBox->setIcon(QIcon(":/warn.png"));
        ui->checkBox->setPalette(pOrange);
        ui->NOWbtn->show();
        ui->description->setText(info.description);
        ui->description->setPalette(pOrange);
        break;
    case ready:
        if (iconFile->exists()) ui->checkBox->setIcon(provider.icon(*iconFile));
        ui->checkBox->setPalette(pDefault);
        ui->NOWbtn->show();
        ui->description->setText(info.description);
        ui->description->setPalette(pDefault);
    case legacy:
        if (iconFile->exists())
        {
            QIcon tmp = provider.icon(*iconFile);
            QPixmap pix = tmp.pixmap(32,32,QIcon::Disabled);
            ui->checkBox->setIcon(QIcon(pix));
        }
        ui->checkBox->setPalette(pGray);
        ui->NOWbtn->show();
        ui->description->setText(info.description);
        ui->description->setPalette(pGray);
        break;
    default:
        SDDebugMessage("appBox::stopInstall(STATE st)","Coder is invalid!\nInvalid use appBox::stopInstall(STATE st)",true,iconerror);
        break;
    }
}

void appBox::on_pushButton_clicked()
{
    if (ui->extInfo->isHidden())
    {
        ui->extInfo->show();
        ui->pushButton->setText(QString::fromUtf8("Скрыть"));
        AppSettings->beginGroup("View");
        this->setFixedHeight(AppSettings->value("AppBoxHeight").toInt());
        AppSettings->endGroup();
    }
    else
    {
        ui->extInfo->hide();
        ui->pushButton->setText(QString::fromUtf8("Подробнее"));
        this->setFixedHeight(80);
    }
}


void appBox::clearEmptyCommands()
{
    QStringList tmp;
    foreach(QString t,info.commands)
    {
        if (!t.isEmpty()) tmp.push_back(t);
    }
    info.commands = tmp;

}

void appBox::updateMovie()
{
    ui->checkBox->setIcon(QIcon(movie->currentPixmap()));
}

void appBox::setMessage(QString message)
{
    ui->checkBox->setToolTip(ui->checkBox->toolTip()+"\n"+message);
}

void appBox::clearEmptyCommandsAndReverse()
{
    QStringList tmp;
    foreach(QString t,info.commands)
    {
        if (!t.isEmpty()) tmp.push_front(t);
    }
    info.commands = tmp;
}

void appBox::on_NOWbtn_clicked()
{
    emit now(this);
}

void appBox::setUnchecked(bool check)
{
    setChecked(!check);
}

void appBox::setChecked(bool check)
{
    ui->checkBox->setChecked(info.isChecked = check);
    if (check)
    {
        state = ready;
        if (info.depends.size())
        {
            emit dependsNeed(info.depends);
        }
        if (info.conflicts.size())
        {
            emit conflictsCheck(info.conflicts,info.name);
        }
        if (info.isAvir)
        {
            emit avirChecked(this);
        }
        if (info.addons.size())
        {
            //TODO
        }
    }
}

bool appBox::isChecked()
{
    return info.isChecked;
}

void appBox::openUrl(QString url)
{
    QDesktopServices::openUrl(QUrl::fromUserInput(url));
}
