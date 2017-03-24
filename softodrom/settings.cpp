#include "settings.h"
#include "ui_settings.h"
#include <QSettings>
#include <QMessageBox>
#include "softodromglobal.h"

Settings::Settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    init();
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_exitBtn_clicked()
{
    this->close();
}

void Settings::on_pushButton_clicked()
{
    if (AppSettings->isWritable())
    {
        AppSettings->clear();
        AppSettings->setValue("SoftStartupFolder",ui->softFolderEdit->text());
        AppSettings->setValue("UpdateStartupFolder",ui->updatefolderEdit->text());
        AppSettings->setValue("DRPDir",ui->DRPFolderEdit->text());
        AppSettings->setValue("RecursiveSearch",ui->recursiveSearchBox->isChecked());
        AppSettings->setValue("UseAuto.bat",ui->useAutoBatBox->isChecked());
        AppSettings->setValue("UseAuto.cmd",ui->useAutoCmdBox->isChecked());
        AppSettings->setValue("UseInfo.txt",ui->useInfoTxtBox->isChecked());
        AppSettings->setValue("UseIconsFromExe",ui->useIconsFromExeBox->isChecked());
        AppSettings->setValue("IgnoreDuplicate",ui->ignoreDuplicateBox->isChecked());
        AppSettings->setValue("Info.txtIsPrefer",ui->infotxtIsPreferBox->isChecked());
        AppSettings->setValue("IngnoreBathOnly",ui->ignoreBachOnlyBox->isChecked());
        AppSettings->setValue("SmartOsFolder",ui->osNameFolderBox->isChecked());
        AppSettings->setValue("AddCurrentPath",ui->addPathCheckBox->isChecked());
        AppSettings->setValue("AddPathGenWarn",!ui->pathGenWarn->isChecked());
        AppSettings->setValue("DebugMode",ui->debugBox->isChecked());
        AppSettings->beginGroup("View");
        AppSettings->setValue("ShowInfoBlock",ui->ShowInfoBlockBox->isChecked());
        AppSettings->setValue("ShowProgressBars",ui->showProgressBarBox->isChecked());
        AppSettings->setValue("AppBoxWidth",ui->AppBoxWidthSpin->value());
        AppSettings->setValue("AppBoxHeight",ui->AppBoxHSpin->value());
        AppSettings->setValue("ShowConsole",ui->consoleBox->isChecked());
        if (!AppSettings->value("ShowConsole").toBool()) hideConsole();
        emit showInfo(AppSettings->value("ShowInfoBlock").toBool());
        AppSettings->endGroup();
        AppSettings->sync();
    }
    else
    {
        QMessageBox::critical(this,QString::fromUtf8("Файл настроек недоступен для записи!"),QString::fromUtf8("Нет возможности перезаписать конфиг, возможно приложение запущено без административных привелегий или нет прав на запись в папку с программой. Если приложение работает по сети попробуйте запустить его локально от имени администратора и изменить настройки. Путь к файлу %1").arg(AppSettings->fileName().replace("/","\\")), QString::fromUtf8("Угу"));
        //this->close();
        init();
    }
}

void Settings::on_debugBox_clicked(bool checked)
{
    if (!checked) return;

    QMessageBox *box = new QMessageBox(QMessageBox::Question,
                                       QString::fromUtf8("Включить отладку?"),
                                       QString::fromUtf8("Вы дествительно хотите включить отладку? \n"
                                                         "В режиме отладки программа будет сыпать неимоверным количеством сообщений! \n"
                                                         "Использовать её по назначению в этом режиме практически невозможно!"),
                                       QMessageBox::Yes|QMessageBox::No);
    box->button(QMessageBox::Yes)->setText(QString::fromUtf8(" ДА! Я знаю что делаю. "));
    box->button(QMessageBox::No)->setText(QString::fromUtf8(" НЕТ! Верните как было. "));
    if (box->exec() == QMessageBox::No)
        ui->debugBox->setChecked(false);
}

void Settings::init()
{
    if (AppSettings->group() == "View")
    {
        AppSettings->endGroup();
        SDDebugMessage("Settings::init()",QString::fromUtf8("AppSettings->group() = View при попытке писать в General"),false,iconwarning);
    }
    ui->softFolderEdit->setText(AppSettings->value("SoftStartupFolder").toString());
    ui->updatefolderEdit->setText(AppSettings->value("UpdateStartupFolder").toString());
    ui->DRPFolderEdit->setText(AppSettings->value("DRPDir").toString());
    ui->recursiveSearchBox->setChecked(AppSettings->value("RecursiveSearch").toBool());
    ui->useAutoBatBox->setChecked(AppSettings->value("UseAuto.bat").toBool());
    ui->useAutoCmdBox->setChecked(AppSettings->value("UseAuto.cmd").toBool());
    ui->useInfoTxtBox->setChecked(AppSettings->value("UseInfo.txt").toBool());
    ui->useIconsFromExeBox->setChecked(AppSettings->value("UseIconsFromExe").toBool());
    ui->ignoreDuplicateBox->setChecked(AppSettings->value("IgnoreDuplicate").toBool());
    ui->infotxtIsPreferBox->setChecked(AppSettings->value("Info.txtIsPrefer").toBool());
    ui->ignoreBachOnlyBox->setChecked(AppSettings->value("IngnoreBathOnly").toBool());
    ui->osNameFolderBox->setChecked(AppSettings->value("SmartOsFolder").toBool());
    ui->addPathCheckBox->setChecked(AppSettings->value("AddCurrentPath").toBool());
    ui->pathGenWarn->setChecked(!AppSettings->value("AddPathGenWarn").toBool());
    ui->pathGenWarn->setEnabled(ui->addPathCheckBox->isChecked());
    ui->debugBox->setChecked(AppSettings->value("DebugMode").toBool());
    AppSettings->beginGroup("View");
    ui->ShowInfoBlockBox->setChecked(AppSettings->value("ShowInfoBlock").toBool());
    ui->showProgressBarBox->setChecked(AppSettings->value("ShowProgressBars").toBool());
    ui->AppBoxWidthSpin->setValue(AppSettings->value("AppBoxWidth").toInt());
    ui->AppBoxHSpin->setValue(AppSettings->value("AppBoxHeight").toInt());
    ui->consoleBox->setChecked(AppSettings->value("ShowConsole").toBool());
    AppSettings->endGroup();
}


void Settings::on_consoleBox_clicked(bool checked)
{
    if (!checked) return;
    QMessageBox::information(this,QString::fromUtf8("Консоль запускается только при старте!"),QString::fromUtf8("Консоль будет отображена только при следующем запуске приложения, если нужно получить отладочный вывод сейчас снимите этот флаг и используйте отладку. Не забудьте переключить приложение в обычный режим после устранения проблем."),QString::fromUtf8("Угу"));
}
