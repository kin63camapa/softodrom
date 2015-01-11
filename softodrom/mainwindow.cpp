#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QSettings>
#include <QDesktopServices>
#include <QUrl>
#include "softodromglobal.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    installOneNow = false;
    autoSelectorWorking = true;
    setWindowIcon(QIcon(":/softodrom.ico"));
    qRegisterMetaType<appInfo>("appInfo");
    ui->setupUi(this);
    settings = new Settings();
    connect(settings,SIGNAL(showInfo(bool)),this,SLOT(showInfo(bool)));
    this->setWindowTitle(QString::fromUtf8("Софтодром"));
    ui->showButton->hide();
    AppSettings->beginGroup("View");
    if (!AppSettings->value("ShowInfoBlock").toBool()) ui->infoBox->hide();
    AppSettings->endGroup();
    this->showMaximized();
    scaner = new Scaner(this);
    connect(scaner,SIGNAL(finished()),this,SLOT(scanComplete()));
    connect(scaner,SIGNAL(terminated()),this,SLOT(scanComplete()));
    connect(scaner,SIGNAL(destroyed()),this,SLOT(scanComplete()));
    connect(scaner,SIGNAL(sizeProgBar(int)),this,SLOT(initSoftProgBar(int)));
    connect(scaner,SIGNAL(progBarPlus()),this,SLOT(incSoftProgBar()));
    connect(scaner,SIGNAL(sizeProgBar(int)),this,SLOT(initUpdProgBar(int)));
    connect(scaner,SIGNAL(progBarPlus()),this,SLOT(incUpdProgBar()));
    connect(scaner,SIGNAL(app(appInfo)),this,SLOT(newApp(appInfo)));

    installer = new Installer(this);
    connect(installer,SIGNAL(result(appBox*,appBox::STATE)),
            this,SLOT(result(appBox*,appBox::STATE)));
    connect(installer,SIGNAL(finished()),this,SLOT(installComplete()));
    connect(installer,SIGNAL(terminated()),this,SLOT(installComplete()));
    connect(installer,SIGNAL(destroyed()),this,SLOT(installComplete()));

    showAllAction =  new QAction(QString::fromUtf8("Показать все"),this);
    connect(showAllAction,SIGNAL(triggered()),this,SLOT(showAll()));
    hideAllAction = new QAction(QString::fromUtf8("Скрыть все"),this);
    connect(hideAllAction,SIGNAL(triggered()),this,SLOT(hideAll()));
    ui->menu->addAction(showAllAction);
    ui->menu->addAction(hideAllAction);
    ui->menu->addSeparator();
    connect(ui->unmarkedInstalledAction,SIGNAL(triggered()),this,SLOT(uncheckInstalled()));
    connect(ui->startSetupAction,SIGNAL(triggered()),this,SLOT(on_startStopButton_clicked()));
    connect(ui->rescanAction,SIGNAL(triggered()),this,SLOT(on_renewButton_clicked()));
    connect(ui->settingsAction,SIGNAL(triggered()),this,SLOT(showSettings()));
    aboutBox = new AboutBox();
    if (!QFileInfo(AppSettings->value("DRPDir").toString()+QDir::separator()+"DriverPackSolution.exe").exists())
        ui->drpOpenBtn->hide();
    rescanApps();
    QString infoText;
    QTextStream(&infoText) << QString::fromUtf8("Операционная система: Windows ");
    if (OSinfo.isServer)
        QTextStream(&infoText) << "Server ";
    QTextStream(&infoText) << OSinfo.toString(QString::fromUtf8("%Name% "));
    if (OSinfo.ServicePackMajor)
        QTextStream(&infoText) << OSinfo.toString("Service Pack %ServicePackMajor% ");
    QTextStream(&infoText) << OSinfo.toString(QString::fromUtf8("%Bits% разрядная build %MajorVersion%.%MinorVersion%.%BuildNumber%"));
    if (OSinfo.ServicePackMajor || OSinfo.ServicePackMinor)
        QTextStream(&infoText) << OSinfo.toString(" SP %ServicePackMajor%.%ServicePackMinor%");
    if (OSinfo.szCSDVersion.size())
        QTextStream(&infoText) << " " << OSinfo.szCSDVersion;
    if (OSinfo.toString("%SuiteMask%") != "%SuiteMask%")
        QTextStream(&infoText) << OSinfo.toString(" %SuiteMask%");
    QTextStream(&infoText) << "\n";
    if (OSinfo.isAdmin)
        QTextStream(&infoText) << QString::fromUtf8("Права администратора\n");
    else
        QTextStream(&infoText) << QString::fromUtf8("Ограниченные права\n");
    if (AppSettings->isWritable())
        QTextStream(&infoText) << QString::fromUtf8("Файл настроек доступен для записи\n");
    else
        QTextStream(&infoText) << QString::fromUtf8("Файл настроек для записи не доступен\n");
    if (OSinfo.is64build)
        QTextStream(&infoText) << QString::fromUtf8("64 разрядная сборка\n");
    else
        QTextStream(&infoText) << QString::fromUtf8("32 разрядная сборка\n");
    ui->InfoLabel->setText(infoText);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showInfo(bool show)
{
    if (show) ui->infoBox->show();
    else ui->infoBox->hide();

}

void MainWindow::rebulildBoxes()
{
    AppSettings->beginGroup("View");
    int tmp = AppSettings->value("AppBoxWidth").toInt();
    AppSettings->endGroup();
    if (tmp) tmp = (ui->softArea->frameSize().width()-19*2-(ui->softLayout->columnCount()-1)*2)/tmp;
    if (!tmp) tmp = 1;
    //tmp = 1; //луна твердая! KIN.
    if (false/*col == tmp*/)
    {
        return;
    }else
    {
        col = tmp;
        int r = 0,c = 0;
        if (soft.size())
        {
            ui->softLayout->removeItem(ui->verticalSpacer);
            foreach (appBox *box, soft)
            {
                ui->softLayout->removeWidget(box);
                ui->softLayout->addWidget(box,r,c++);
                if (c==col) {c = 0;r++;}
            }
            ui->softLayout->addItem(ui->verticalSpacer,r+2,0);
        }
        r=0;
        c=0;
        if (update.size())
        {
            ui->updateLayout->removeItem(ui->verticalSpacer_2);
            foreach (appBox *box, update)
            {
                ui->updateLayout->removeWidget(box);
                 if (!box->isHidden()) ui->updateLayout->addWidget(box,r,c++);
                if (c==col) {c = 0;r++;}
            }
            ui->updateLayout->addItem(ui->verticalSpacer_2,r+2,0);
        }
    }
}

void MainWindow::rescanApps()
{
    ui->softLayout->removeItem(ui->verticalSpacer);
    foreach (appBox *box, soft){ ui->softLayout->removeWidget(box); delete box;}
    ui->softLayout->addItem(ui->verticalSpacer,1,0);
    ui->updateLayout->removeItem(ui->verticalSpacer_2);
    foreach (appBox *box, update){ ui->updateLayout->removeWidget(box); delete box;}
    ui->updateLayout->addItem(ui->verticalSpacer_2,1,0);
    soft.clear();
    update.clear();
    ui->renewButton->setText(QString::fromUtf8(" Остановить сканирование "));
    ui->rescanAction->setText(QString::fromUtf8("Остановить сканирование"));
    ui->startSetupAction->setDisabled(1);
    ui->startStopButton->setDisabled(1);

    AppSettings->beginGroup("View");
    if (AppSettings->value("ShowProgressBars").toBool())
    {

        ui->softProgressBar->setFormat(QString::fromUtf8("  Поиск приложений %p%"));
        ui->updProgressBar->setFormat(QString::fromUtf8("  Поиск обновлений %p%"));
        ui->softProgressBar->setValue(0);
        ui->updProgressBar->setValue(0);
        ui->softProgressBar->show();
        ui->updProgressBar->show();
        ui->noSoftLabel->hide();
        ui->noUpdateLabel->hide();
    }else
    {
        ui->softProgressBar->hide();
        ui->updProgressBar->hide();
        ui->noSoftLabel->setText(QString::fromUtf8("Поиск приложений..."));
        ui->noUpdateLabel->setText(QString::fromUtf8("Поиск обновлений..."));
        ui->noSoftLabel->show();
        ui->noUpdateLabel->show();
    }
    AppSettings->endGroup();
    scaner->start(QThread::LowPriority);
}

void MainWindow::scanComplete()
{
    ui->softProgressBar->hide();
    if (soft.size())
    {
        ui->noSoftLabel->hide();
    }else{
        ui->noSoftLabel->setText(QString::fromUtf8("Нет программ для установки"));
        ui->noSoftLabel->show();
    }
    ui->updProgressBar->hide();
    if (update.size())
    {
        ui->noUpdateLabel->hide();
    }else{
        ui->noUpdateLabel->setText(QString::fromUtf8("Нет доступных обновлений"));
        ui->noUpdateLabel->show();
    }
    ui->renewButton->setText(QString::fromUtf8(" Обновить список "));
    ui->rescanAction->setText(QString::fromUtf8("Обновить список"));
    col = 0;
    foreach(KitMenu * tmpMenu, kitsMenus)
    {
        disconnect(tmpMenu,SIGNAL(enable(QString)),this,SLOT(enableKit(QString)));
        disconnect(tmpMenu,SIGNAL(disable(QString)),this,SLOT(disableKit(QString)));
        disconnect(tmpMenu,SIGNAL(setMarked(QString)),this,SLOT(markedKit(QString)));
        disconnect(tmpMenu,SIGNAL(setUnmarked(QString)),this,SLOT(unmarkedKit(QString)));
        delete tmpMenu;
    }
    kitsMenus.clear();
    KitMenu * tmpMenu;
    kits.removeDuplicates();
    foreach(QString kit, kits)
    {
        tmpMenu = new KitMenu(ui->menu);
        tmpMenu->setKitName(kit);
        ui->menu->addMenu(tmpMenu);
        connect(tmpMenu,SIGNAL(enable(QString)),this,SLOT(enableKit(QString)));
        connect(tmpMenu,SIGNAL(disable(QString)),this,SLOT(disableKit(QString)));
        connect(tmpMenu,SIGNAL(setMarked(QString)),this,SLOT(markedKit(QString)));
        connect(tmpMenu,SIGNAL(setUnmarked(QString)),this,SLOT(unmarkedKit(QString)));
        kitsMenus.push_back(tmpMenu);
    }
    rebulildBoxes();
    ui->startSetupAction->setDisabled(0);
    ui->startStopButton->setDisabled(0);
    on_unmarkedAllAction_triggered();
    markedKit("base");
    uncheckInstalled();
    autoSelectorWorking = false;
}

void MainWindow::on_hideButton_clicked()
{
    ui->infoBox->hide();
    ui->showButton->show();
}

void MainWindow::on_showButton_clicked()
{
    ui->infoBox->show();
    ui->showButton->hide();
}

void MainWindow::result(appBox * box, appBox::STATE status)
{
    box->stopInstall(status);
}


void MainWindow::showSettings()
{
    settings->show();
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    event->accept();
    rebulildBoxes();
}

void MainWindow::initSoftProgBar(int size)
{
    ui->softProgressBar->setMaximum(size);
    ui->softProgressBar->setValue(0);
}

void MainWindow::incSoftProgBar()
{
    ui->softProgressBar->setValue(ui->softProgressBar->value()+1);
}

void MainWindow::initUpdProgBar(int size)
{
    ui->updProgressBar->setMaximum(size);
    ui->updProgressBar->setValue(0);
}

void MainWindow::incUpdProgBar()
{
    ui->updProgressBar->setValue(ui->updProgressBar->value()+1);
}

void MainWindow::on_renewButton_clicked()
{
    if (scaner->isRunning())
    {
        scaner->terminate();
        scanComplete();
    }
    else rescanApps();
}

void MainWindow::newApp(appInfo app)
{
    appBox *tmp;
    tmp = new appBox();
    tmp->setInfo(app);
    kits += tmp->getInfo().kits;
    connect(tmp,SIGNAL(now(appBox*)),this,SLOT(installOne(appBox*)));
    connect(tmp,SIGNAL(avirChecked(appBox*)),this,SLOT(avirChecked(appBox*)));
    connect(tmp,SIGNAL(conflictsCheck(QStringList,QString)),this,SLOT(conflictsCheck(QStringList,QString)));
    connect(tmp,SIGNAL(dependsNeed(QStringList)),this,SLOT(dependsNeed(QStringList)));
    if (tmp->getInfo().isAvir)
    {
        //TODO проверка антивируса
        ;
    }
    soft.push_back(tmp);
}

void MainWindow::closeEvent(QCloseEvent *e)
{    
    if (scaner->isRunning() || installer->isRunning())
    {
        QMessageBox::StandardButton q;
        q = QMessageBox::question(this, QString::fromUtf8("Закрыть программу"), QString::fromUtf8("Идёт процесс поиска или установки приолжений, вы действительно хотите принудительно остановить его и выйти из программы?"),
                                  QMessageBox::Yes|QMessageBox::No);
        if (q == QMessageBox::Yes)
        {
            if (scaner->isRunning())scaner->terminate();
            if (installer->isRunning())installer->terminate();
            e->accept();
        }
        else
        {
            e->ignore();
        }
    }
    else
    {
        e->accept();
    }
}

void MainWindow::installOne(appBox *app)
{
    if (installer->isRunning())
    {
        QMessageBox::information(this,QString::fromUtf8("Программа занята"),QString::fromUtf8("В настоящий момент уже выполняется установка, дождитесь окончания или перезапустите программу если процесс завис."), QString::fromUtf8("Угу"));
        return;
    }
    if (!app->getInfo().commands.size())
    {
        QMessageBox::information(this,QString::fromUtf8("Ошибочка вышла :("),
                                 QString::fromUtf8("Как-то так получилось что список комманд для программы %1 пуст! \n"
                                                   "Проверьте есть ли у вас версия под текущую платформу и правильно ли составлен файл описания. \n"
                                                   "Папка с программой %2").arg(app->getInfo().name).arg(app->getInfo().dir), QString::fromUtf8("Угу"));
        return;
    }
    installOneNow = 1;
    ui->renewButton->setDisabled(true);
    ui->rescanAction->setDisabled(true);
    app->startWait();
    QList <appBox*> l;
    l.push_back(app);
    installer->setApps(l);
    installer->start(QThread::HighPriority);
}

void MainWindow::on_startStopButton_clicked()
{
    if (installOneNow)
    {
        QMessageBox::information(this,"Error",QString::fromUtf8("В настоящий момент уже выполняется установка, дождитесь окончания или перезапустите программу если процесс завис."), QString::fromUtf8("Угу"));
        return;
    }
    if (installer->isRunning())
    {
        while (installer->isRunning())
            installer->terminate();
    }
    else
    {
        ui->renewButton->setDisabled(true);
        ui->rescanAction->setDisabled(true);
        ui->startStopButton->setText(QString::fromUtf8(" Прервать установку "));
        ui->startSetupAction->setText(QString::fromUtf8("Прервать установку"));
        QList <appBox*> checkedApps;
        foreach(appBox *tmp,soft)
        {
            if (tmp->isChecked() && tmp->state == appBox::ready)
            {
                tmp->startWait();
                checkedApps.push_back(tmp);
            }else
            {
                tmp->setUnchecked();
                tmp->hide();
            }
        }
        installer->setApps(checkedApps);
        installer->start(QThread::HighPriority);
    }

    if (!installer->isRunning())
    {
        installOneNow = 0;
        ui->startStopButton->setText(QString::fromUtf8(" Начать установку "));
        ui->startSetupAction->setText(QString::fromUtf8("Начать установку"));
        ui->renewButton->setEnabled(true);
        ui->rescanAction->setEnabled(true);
        foreach(appBox *tmp,soft)
        {
            if (tmp->state == appBox::wait)
            {
                tmp->stopInstall(appBox::ready);
            }
            if (tmp->state == appBox::setup)
            {
                tmp->stopInstall(appBox::error);
                tmp->setMessage(QString::fromUtf8("Установка прервана!"));
            }
        }
    }
}

void MainWindow::installComplete()
{
    installOneNow = false;
    if (installer->isRunning()) installer->terminate();
    ui->renewButton->setEnabled(true);
    ui->rescanAction->setEnabled(true);
    ui->startStopButton->setText(QString::fromUtf8(" Начать установку "));
    ui->startSetupAction->setText(QString::fromUtf8("Начать установку"));
    QMessageBox::information(this,QString::fromUtf8("Уcтановка завершена!"),QString::fromUtf8("Уcтановка завершена, для просмотра статуса неведите курсор на название программы."),QString::fromUtf8("Угу"));
}

void MainWindow::enableKit(QString kitname)
{
    foreach(appBox* tmp,soft)
    {
        if (tmp->getInfo().kits.contains(kitname))
        {
            tmp->show();
            //tmp->setChecked();
        }
    }
}

void MainWindow::disableKit(QString kitname)
{
    foreach(appBox* tmp,soft)
    {
        if (tmp->getInfo().kits.contains(kitname))
        {
            tmp->hide();
            tmp->setUnchecked();
        }
    }
    rebulildBoxes();
}

void MainWindow::markedKit(QString kitname)
{
    foreach(appBox* tmp,soft)
    {
        if (tmp->getInfo().kits.contains(kitname) && tmp->state == appBox::ready)
        {
            tmp->setChecked();
        }
    }
}

void MainWindow::unmarkedKit(QString kitname)
{
    foreach(appBox* tmp,soft)
    {
        if (tmp->getInfo().kits.contains(kitname))
        {
            tmp->setUnchecked();
        }
    }
}

void MainWindow::uncheckInstalled()
{
    foreach(appBox* tmp,soft)
    {
        if (tmp->state == appBox::normal)
        {
            tmp->setUnchecked();
        }
    }
    foreach(appBox* tmp,update)
    {
        if (tmp->state == appBox::normal)
        {
            tmp->setUnchecked();
        }
    }
}

void MainWindow::on_markedAllAction_triggered()
{
    foreach(appBox* tmp,soft)
    {
        if (!tmp->isHidden())
        {
            tmp->setChecked();
        }
    }
    foreach(appBox* tmp,update)
    {
        if (!tmp->isHidden())
        {
            tmp->setChecked();
        }
    }
}

void MainWindow::on_unmarkedAllAction_triggered()
{
    foreach(appBox* tmp,soft)
    {
        tmp->setUnchecked();
    }
    foreach(appBox* tmp,update)
    {
        tmp->setChecked();
    }
}

void MainWindow::on_markedSoftOnly_triggered()
{
    foreach(appBox* tmp,soft)
    {
        tmp->setChecked();
    }
    foreach(appBox* tmp,update)
    {
        tmp->setUnchecked();
    }
}

void MainWindow::on_markedUpdatesOnly_triggered()
{
    foreach(appBox* tmp,soft)
    {
        tmp->setUnchecked();
    }
    foreach(appBox* tmp,update)
    {
        tmp->setChecked();
    }
}

void MainWindow::showAll()
{
    foreach(appBox* tmp,soft)
    {
        tmp->show();
    }
}

void MainWindow::hideAll()
{
    foreach(appBox* tmp,soft)
    {
        tmp->hide();
        tmp->setUnchecked();
    }
}

void MainWindow::on_actionHelp_triggered()
{
    bool helpNoWritten = 0;
    QDir d(QCoreApplication::applicationDirPath()+QDir::separator()+"help");
    d.makeAbsolute();
    SDDebugMessage("void MainWindow::on_actionHelp_triggered()",d.absolutePath());
    if (d.exists() && d.entryList(QDir::Files).size())
        helpNoWritten = !QDesktopServices::openUrl(QUrl::fromUserInput(QCoreApplication::applicationDirPath()+QDir::separator()+"help"+QDir::separator()+d.entryList(QDir::Files).at(0)));
    else
        helpNoWritten++;
    if (helpNoWritten)
        QMessageBox::information(this,"Warning",QString::fromUtf8("Похоже документацию так никто и не написал... можете почитать исходники.\nЕсли напишете положите в папку help рядом с программой"), QString::fromUtf8("Okay..."));
}

void MainWindow::on_actionAbout_triggered()
{
    aboutBox->show();
}

void MainWindow::on_niniteOpenBtn_clicked()
{
    QDesktopServices::openUrl(QUrl::fromUserInput("https://ninite.com/"));
}

void MainWindow::on_drpOpenBtn_clicked()
{
    SDRunExternal(AppSettings->value("DRPDir").toString()+QDir::separator()+"DriverPackSolution.exe");
}

void MainWindow::avirChecked(appBox *app)
{
    foreach(appBox* tmp,soft)
    {
        if (tmp->getInfo().kits.contains("avir"))
        {
            if (tmp != app && tmp->isChecked())
            {
                tmp->setUnchecked();
                if (!autoSelectorWorking)
                    QMessageBox::information(
                                this,
                                QString::fromUtf8("Контроль антивирусов"),
                                QString::fromUtf8("Обратите внимание в систему можно установить только 1 антивирус!\n"
                                                  "Так как для установки был отмечен продукт %1,\n"
                                                  "Установка %2 была отменена!").arg(app->getInfo().name).arg(tmp->getInfo().name),
                                QString::fromUtf8("Угу"));
            }
        }
    }
}

void MainWindow::conflictsCheck(QStringList conflicts,QString name)
{
    foreach(appBox* tmp,soft)
    {
        foreach (QString tmpName, conflicts)
        {
            if (tmp->isChecked() && (tmpName == tmp->getInfo().name))
            {
                tmp->setUnchecked();
                if (!autoSelectorWorking)
                    QMessageBox::information(
                                this,
                                QString::fromUtf8("Контроль конфликтов"),
                                QString::fromUtf8("Обратите внимание: приложение %1 \n"
                                                  "конфликтует с %2,\n"
                                                  "Установка %2 была отменена!").arg(name).arg(tmpName),
                                QString::fromUtf8("Угу"));
            }
        }
    }
}

void MainWindow::dependsNeed(QStringList depends)
{
    ;
}
