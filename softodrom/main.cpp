#ifdef WIN64
#define BITS 64
#else
#define BITS 32
#endif

#include <QtGui/QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QPushButton>
#include <QStyle>
#include "softodromglobal.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.addLibraryPath(a.applicationDirPath()+QDir::separator()+"dll"+QString::number(BITS));
    a.setEffectEnabled(Qt::UI_AnimateMenu);
    a.setEffectEnabled(Qt::UI_FadeMenu);
    a.setEffectEnabled(Qt::UI_AnimateCombo);
    a.setEffectEnabled(Qt::UI_AnimateTooltip);
    a.setEffectEnabled(Qt::UI_FadeTooltip);
    a.setEffectEnabled(Qt::UI_AnimateToolBox);
    a.setApplicationName("softodrom");
    a.setApplicationVersion("0.4.1");
    a.setOrganizationName("SmartTech");
    a.setOrganizationDomain("smart-tech.biz");
    a.setAttribute(Qt::AA_DontShowIconsInMenus);
    a.setGraphicsSystem("raster");

    QMessageBox *box = new QMessageBox(QMessageBox::Warning,
                          QString::fromUtf8("Проблемы на старте :("),
                          QString::fromUtf8(""),
                          QMessageBox::Yes|QMessageBox::No|QMessageBox::Retry);
    box->button(QMessageBox::Retry)->setText(QString::fromUtf8(" Перезапуск c повышением "));
    box->button(QMessageBox::Yes)->setText(QString::fromUtf8(" ДА! Продолжать в текущем режиме. "));
    box->button(QMessageBox::No)->setText(QString::fromUtf8(" НЕТ! Выходим. "));
    box->setIconPixmap(QIcon(":/softodrom.ico").pixmap(48,48));
    box->setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_MessageBoxQuestion));
    AppSettings = new QSettings((QCoreApplication::applicationDirPath()+QDir::separator()+"softodrom.ini"),QSettings::IniFormat);
    if (!AppSettings->contains("SoftStartupFolder"))
    {
        if (!AppSettings->isWritable())
        {
            box->setText(QString::fromUtf8("Не найден работоспособный конфиг, и нет возможности его создать! \n"
                                  "Попробовать продолжить с настройками по умолчанию?"));
            int res = box->exec();
            if (res == QMessageBox::No)
                return 5;
            if (res == QMessageBox::Retry)
            {
                SDRunExternal(QCoreApplication::applicationFilePath(),"RunWithElevation");
                return 0;
            }
        }
    }
    setDefaultConfig(false);
    if (AppSettings->isWritable()) AppSettings->sync();
    AppSettings->beginGroup("View");
    if (!AppSettings->value("ShowConsole").toBool()) hideConsole();
    AppSettings->endGroup();
    if (!OSinfo.init())
    {
        if (a.arguments().count("RunWithElevation")) box->button(QMessageBox::Retry)->hide();
        else
        {
            /*if (box->button(QMessageBox::Retry)->isHidden())*/ box->button(QMessageBox::Retry)->show();
            box->button(QMessageBox::Retry)->setText(QString::fromUtf8(" Перезапуск c повышением "));
        }
        box->setText(QString::fromUtf8("<center><h3><br>Ошибка определения оболочки !?</h3></center>"
                                       "Собственно... похоже программа не сумела определить версию OS. \n"
                                       "Такое могло произойти если вы запустили программу в нестандартной среде,"
                                       "операционная система серьезно повреждена или заражена вирусами!"
                                       "Сейчас мы можем попробовать продолжить в текущем режиме, однако, скорее всего "
                                       "Программа будет работать непредсказуемо!"
                                       ));
        int res = box->exec();
        if (res == QMessageBox::No)
            return 5;
        if (res == QMessageBox::Retry){
            SDRunExternal(QCoreApplication::applicationFilePath(),
                             "RunWithElevation",
                             QCoreApplication::applicationDirPath()+QDir::separator()+OSinfo.toString("dll%Bits%"));
            return 0;
        }
    };
    if (OSinfo.is64 != OSinfo.is64build)
    {
        if (a.arguments().count("Run64")) box->button(QMessageBox::Retry)->hide();
        else box->button(QMessageBox::Retry)->setText(QString::fromUtf8(" Перезапуск в X64 "));
        box->setText(QString::fromUtf8("<center><h3><br>Архитектура OS и приложения не совпадают</h3></center>"
                                       "Собственно... похоже в 64 разрядной среде запущена 32 разрядная версия программы.\n"
                                       "Такое могло произойти если вы запустили программу без помощи лаунчера. "
                                       "Сейчас мы можем попробовать продолжить в текущем режиме, однако, программа"
                                       " не сможет правильно работать с реестром и переменными среды!"
                                       ));
        int res = box->exec();
        if (res == QMessageBox::No)
            return 5;
        if (res == QMessageBox::Retry){
            SDRunExternal(QCoreApplication::applicationDirPath()+QDir::separator()+"softodrom64",
                             "Run64 RunWithElevation",QCoreApplication::applicationDirPath()+QDir::separator()+"dll64");
            return 0;
        }
    }
    if (!OSinfo.isAdmin)
    {
        if (a.arguments().count("RunWithElevation")) box->button(QMessageBox::Retry)->hide();
        else
        {
            /*if (box->button(QMessageBox::Retry)->isHidden())*/ box->button(QMessageBox::Retry)->show();
            box->button(QMessageBox::Retry)->setText(QString::fromUtf8(" Перезапуск c повышением "));
        }
        box->setText(QString::fromUtf8("<center><h3><br>Тварь я дрожащая или право имею !?</h3></center>"
                                       "Собственно... похоже программа запущена без прав администратора. \n"
                                       "Такое могло произойти если вы запустили программу от пользователя не имеющего административных прав, "
                                       "запустили программу без помощи лаунчера или UAC в вашей системе дал сбой. "
                                       "Сейчас мы можем попробовать продолжить в текущем режиме, однако, скорее всего "
                                       "ни о какой корректной автоматической установке речи быть не может: "
                                       "программа завалит вас окнами UAC (в лучшем случае) или скриптам прото не хватит прав на корректную установку!"
                                       ));
        int res = box->exec();
        if (res == QMessageBox::No)
            return 5;
        if (res == QMessageBox::Retry){
            SDRunExternal(QCoreApplication::applicationFilePath(),
                             "RunWithElevation",
                             QCoreApplication::applicationDirPath()+QDir::separator()+OSinfo.toString("dll%Bits%"));
            return 0;
        }

    }
    delete box;
    MainWindow w;
    w.show();
    return  a.exec();
}
