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
    list.clear();
    list = apps;
}

HANDLE Installer::SDRunExternalEx(QString cmd, QString dir,appBox *box)
{
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
    unsigned long waitResult;
    DWORD exitCode;
    HANDLE process;
    QString arg;

    foreach(appBox *box,list)
    {
        box->startInstall();
        box->setMessage("\n");
        status = appBox::normal;
        foreach (QString command, box->getInfo().commands)
        {
            command.replace("%directory%",box->getInfo().dir);
            command.replace("%name%",box->getInfo().name);
            command.replace("%key%",box->getInfo().key);
            command.replace("%sep%",QDir::separator());
            SDDebugMessage("Installer::run()",box->getInfo().dir+"\n"+command);
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
        emit result(box,status);
    }
    emit finish();
}

QString Installer::target(QString command)
{
    return command.left(command.indexOf(" "));
}

QString Installer::args(QString command)
{
    return command.right(command.size()-command.indexOf(" ")-1);
}

QString Installer::dropQuotes(QString command)
{
    if (command.at(0) != '\"')
        return command;
    else
        if (command.at(command.size()-1) != '\"')
            return command;
        else
        {
            command.resize(command.size()-1);
            return command.right(command.size()-1);
        }
}
