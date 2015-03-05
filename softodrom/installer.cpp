#include "softodromglobal.h"
#include "installer.h"
#include "windows.h"
#include "errcodes.h"

Installer::Installer(QObject *parent) :
    QThread(parent)
{
    qRegisterMetaType<appBox::STATE>("appBox::STATE");
    status = appBox::normal;
}

void Installer::setApps(QList<appBox *> apps)
{
    SDDebugMessage("Installer::setApps(QList<appBox *> apps)","Started");
    list.clear();
    appBox * avir;
    bool avirExist = false;
    foreach (appBox * tmp , apps)
    {
        if (tmp->getInfo().isAvir)
        {
            avirExist = true;
            avir = tmp;
            continue;
        }
        if (tmp->getInfo().kits.contains("runtimes")) list.push_front(tmp);
        else list.push_back(tmp);
    }
    if (avirExist) list.push_back(avir);
}

HANDLE Installer::SDRunExternalEx(QString cmd, QString dir,appBox *box)
{
    SDDebugMessage(QString("Installer::SDRunExternalEx(%1,%2,%3)").arg(cmd).arg(dir).arg(box->getInfo().name),"Started");
    STARTUPINFO si = {0};
    PROCESS_INFORMATION pi;
    WCHAR command[MAX_PATH*4] = {0};
    WCHAR directory[MAX_PATH] = {0};
    cmd.toWCharArray(command);
    dir.toWCharArray(directory);
    if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, directory, &si, &pi))
    {
        box->setMessage(QString::fromUtf8("Выполнить %1 в %2 не удалось.\n%3.").arg(cmd).arg(dir).arg(code2text(GetLastError())));
        if (AppSettings->value("AddCurrentPath").toBool())
        {
            if (status == appBox::normal) status = appBox::warning;
            QString tmp = dir+QDir::separator()+cmd;
            tmp.toWCharArray(command);
            if (!CreateProcess(NULL, command, NULL, NULL, FALSE, 0, NULL, directory, &si, &pi))
            {
                box->setMessage(QString::fromUtf8("Выполнить %1 в %2 не удалось.\n%3.").arg(cmd).arg(dir).arg(code2text(GetLastError())));
                return NULL;
            }else
            {
                box->setMessage(QString::fromUtf8("Путь %2 к исполняемому файлу %1 подставлен автоматически!").arg(cmd).arg(dir));
                SDDebugMessage(QString("Installer::SDRunExternalEx(%1, %2, %3").arg(cmd).arg(dir).arg(box->getInfo().name),
                               QString::fromUtf8("Путь %2 к исполняемому файлу %1 подставлен автоматически!").arg(cmd).arg(dir),false,iconwarning);
            }
        }
        else return NULL;
    }
    return pi.hProcess;
}

void Installer::run()
{
    SDDebugMessage(QString("Installer::run()"),"Started");
    unsigned long waitResult;
    DWORD exitCode;
    HANDLE process;
    QString arg;
    QString originalDir;
    foreach(appBox *box,list)
    {
        box->startInstall();
        box->setMessage("\n");
        status = appBox::normal;
        if (box->getInfo().mkTmp)
        {
            QString tmpdir = ExpandEnvironmentString("%tmp%")+QDir::separator()+QCoreApplication::applicationName()+"_"+box->getInfo().name+QString::number(QDateTime::currentDateTime().toTime_t());
            if (copyDir(box->getInfo().dir,tmpdir))
            {
                originalDir = box->getInfo().dir;
                box->setDir(tmpdir);
                box->setMessage(QString::fromUtf8("Запуск будет произведен из %1").arg(tmpdir));
            }
            else{
                if (false)
                {
                    status = appBox::warning;
                    box->setMessage(QString::fromUtf8("Скопировать программу в %1 не удалось. Запуск из текущего места...").arg(tmpdir));
                }else
                {
                    box->setMessage(QString::fromUtf8("Скопировать программу в %1 не удалось. Установка прервана!").arg(tmpdir));
                    emit result(box,appBox::error);
                    continue;
                }
            }
        }
        foreach (QString command, box->getInfo().commands)
        {
            command.replace("%directory%",box->getInfo().dir);
            command.replace("%name%",box->getInfo().name);
            command.replace("%key%",box->getInfo().key);
            command.replace("%sep%",QDir::separator());
            command=ExpandEnvironmentString(command);
            SDDebugMessage("Installer::run()",QString("Execute %1").arg(box->getInfo().dir+"\n"+command));
            if (!command.indexOf("mkdir"))
            {
                QStringRef dirP(&command, 5, command.size()-5);
                SDDebugMessage("Installer::run()",QString("Intrnal command mkdir for %1").arg(dirP.toString()));
                QDir dir(dirP.toString());
                QString sub(".");
                if (!dir.isAbsolute())
                {
                    dir.setPath(box->getInfo().dir);
                    sub = dirP.toString();
                }
                if (dir.mkdir(sub)) box->setMessage(QString::fromUtf8("Директория %1 создана.").arg(dir.path()+sub));
                else
                {
                    box->setMessage(QString::fromUtf8("Директорию %1 создать не удалось!").arg(dir.path())+sub);
                    if (status == appBox::normal) status = appBox::warning;
                }
                continue;
            }
            process = SDRunExternalEx(command,box->getInfo().dir,box);
            if (process == NULL)
            {
                status = appBox::error;
                continue;
            }
            if (box->getInfo().killTimer)
                waitResult = WaitForSingleObject(process,box->getInfo().killTimer*1000);
            else
                waitResult = WaitForSingleObject(process,ULONG_MAX);
            switch (waitResult) {
            case WAIT_TIMEOUT:
                SDDebugMessage("Installer::run()","WAIT_TIMEOUT\n"+box->getInfo().dir+"\n"+command);
                TerminateProcess(process, 1);
                status = appBox::error;
                box->setMessage(QString::fromUtf8("Выполнение %1 %2 превысило таймаут в %3 секунд.\nПрервано!").arg(command).arg(arg).arg(box->getInfo().killTimer));
                break;
            case WAIT_OBJECT_0:
                SDDebugMessage("Installer::run()","WAIT_OBJECT_0\n"+box->getInfo().dir+"\n"+command);
                GetExitCodeProcess(process,&exitCode);
                SDDebugMessage("Installer::run()","Exit code for "+command+QString(" is %1").arg(exitCode));
                if (exitCode) status = appBox::error;
                box->setMessage(QString::fromUtf8("Выполнение %1 %2 завершилось с кодом %3.\n%4").arg(command).arg(arg).arg(exitCode).arg(code2text(exitCode)));
                break;
            case WAIT_FAILED:
                SDDebugMessage("Installer::run()",
                              "WAIT_FAILED\n"+box->getInfo().dir+"\n"+command+QString("\n%1").arg(code2text(GetLastError())));
                TerminateProcess(process, 1);
                if (status == appBox::normal) status = appBox::warning;
                box->setMessage(QString::fromUtf8("Выполнить %1 %2 не удалось.\n%3.").arg(command).arg(arg).arg(code2text(GetLastError())));
                break;
            case WAIT_ABANDONED:
                if (status == appBox::normal) status = appBox::warning;
                break;
            default:
                break;
            }
            CloseHandle(process);
        }
        if (box->getInfo().ver.size() && box->getInfo().instVer.size())
        {
            if (box->getInfo().ver == verExpand(box->getInfo().instVer))
            {
                if (status == appBox::error) status = appBox::warning;
                box->setMessage(QString::fromUtf8("Проверка установленной верссии %1 выполнена успешно!").arg(box->getInfo().ver));
            }else
            {
                if (box->getInfo().instVer.size())
                {
                    box->setMessage(QString::fromUtf8("Проверка установленной версии %1 не удалась!").arg(box->getInfo().instVer));
                }
            }
        }
        if (box->getInfo().mkTmp) box->setDir(originalDir);
        emit result(box,status);
    }
}

