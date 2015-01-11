#include <QtCore>
#include <QSettings>
#include "scaner.h"
#include "softodromglobal.h"

Scaner::Scaner(QObject *parent) :
    QThread(parent)
{
    qRegisterMetaType<appInfo>("appInfo");
}

void Scaner::run()
{
    if(AppSettings->value("UseAuto.bat").toBool())indicators.push_back("Auto.bat");
    if(AppSettings->value("UseAuto.cmd").toBool())indicators.push_back("Auto.cmd");
    if(AppSettings->value("UseInfo.txt").toBool())indicators.push_back("Info.txt");
    apps.push_back("*.exe");
    apps.push_back("*.msi");
    filters = apps + indicators;
    QDir startupFolder(AppSettings->value("SoftStartupFolder").toString());
    if (AppSettings->value("SoftStartupFolder").toString().isEmpty())
    {
        SDDebugMessage("Scaner::Scaner()","value SoftStartupFolder is empty!",false,iconwarning);
        startupFolder.setPath(QCoreApplication::applicationDirPath().replace("/","\\")+"\\..\\");
    }
    if (!startupFolder.isAbsolute())
    {
        startupFolder.setPath(QCoreApplication::applicationDirPath()+QDir::separator()+AppSettings->value("SoftStartupFolder").toString());
    }
    startupFolder = QDir(startupFolder.absolutePath());//костыль
    QDir updateFolder = QDir(AppSettings->value("UpdateStartupFolder").toString());
    if (!updateFolder.isAbsolute())
    {
        updateFolder.setPath(QCoreApplication::applicationDirPath()+QDir::separator()+AppSettings->value("UpdateStartupFolder").toString());
    }
    updateFolder = QDir(updateFolder.absolutePath());
    QStringList files = loadFiles(startupFolder,filters);
    emit sizeProgBar(files.size()+1);
    appInfo tmpAppInfo;
    QFileInfo *tmpFileInfo = new QFileInfo("C:\blah\blah\blah.txt");
    QStringList filesInCurrentDir;
    passedDirs.clear();
    passedDirs.push_back(QCoreApplication::applicationDirPath());
    passedDirs.push_back(updateFolder.absolutePath());
    passedDirs.push_back(startupFolder.absolutePath());
    if (AppSettings->value("RecursiveSearch").toBool())
    {
        passedDirs += getTree(updateFolder);
        passedDirs += getTree(QCoreApplication::applicationDirPath());
        passedDirs += getTree(updateFolder.absolutePath());
    }
    foreach (QString file, files)
    {
        emit progBarPlus();
        tmpAppInfo.clear();
        filesInCurrentDir.clear();
        delete tmpFileInfo;
        tmpFileInfo = new QFileInfo(file);
        bool isOlreadyPassed = 0;

        foreach (QString dir, passedDirs)
        {
            //qDebug() << "Chek passed dirs"  << tmpFileInfo->path() << dir;
            if (AppSettings->value("RecursiveSearch").toBool())
            {
                if (!QString::compare(dir,tmpFileInfo->path(),Qt::CaseInsensitive))
                {
                    //qDebug() << "isOlreadyPassed rec";
                    isOlreadyPassed = 1;
                    break;
                }
            }else
            {
                if (dir.contains(tmpFileInfo->path(),Qt::CaseInsensitive)
                        /*|| (tmpFileInfo->path().contains(dir,Qt::CaseInsensitive))*/)
                {
                    //qDebug() << "isOlreadyPassed norec";
                    isOlreadyPassed = 1;
                    break;
                }
            }
        }
        if (isOlreadyPassed) continue;
        if(!tmpFileInfo->dir().dirName().compare("tmp",Qt::CaseInsensitive))
        {
            passedDirs += tmpFileInfo->dir().path();
            continue;
        }
        if(!tmpFileInfo->dir().dirName().compare("temp",Qt::CaseInsensitive))
        {
            passedDirs += tmpFileInfo->dir().path();
            continue;
        }
        if(!tmpFileInfo->dir().dirName().compare("portable",Qt::CaseInsensitive))
        {
            passedDirs += tmpFileInfo->dir().path();
            continue;
        }
        if(!tmpFileInfo->dir().dirName().compare("port",Qt::CaseInsensitive))
        {
            passedDirs += tmpFileInfo->dir().path();
            continue;
        }
        if(!tmpFileInfo->dir().dirName().compare("files",Qt::CaseInsensitive))
        {
            passedDirs += tmpFileInfo->dir().path();
            continue;
        }
        if (AppSettings->value("SmartOsFolder").toBool())
        {
            if(!tmpFileInfo->dir().dirName().compare("2000",Qt::CaseInsensitive)
                    && QSysInfo::windowsVersion() != QSysInfo::WV_2000)
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("XP",Qt::CaseInsensitive)
                    && QSysInfo::windowsVersion() != QSysInfo::WV_XP)
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("2003",Qt::CaseInsensitive)
                    && QSysInfo::windowsVersion() != QSysInfo::WV_2003)
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("vista",Qt::CaseInsensitive)
                    && QSysInfo::windowsVersion() != QSysInfo::WV_VISTA)
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("7",Qt::CaseInsensitive)
                    && QSysInfo::windowsVersion() != QSysInfo::WV_WINDOWS7)
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("win7",Qt::CaseInsensitive)
                    && QSysInfo::windowsVersion() != QSysInfo::WV_WINDOWS7)
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("8",Qt::CaseInsensitive)
                    && OSinfo.toString().compare("6.2",Qt::CaseInsensitive))
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("win8",Qt::CaseInsensitive)
                    && OSinfo.toString().compare("6.2",Qt::CaseInsensitive))
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("8.1",Qt::CaseInsensitive)
                    && OSinfo.toString().compare("6.3",Qt::CaseInsensitive))
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("win8.1",Qt::CaseInsensitive)
                    && OSinfo.toString().compare("6.3",Qt::CaseInsensitive))
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("10",Qt::CaseInsensitive)
                    && OSinfo.toString().compare("6.4",Qt::CaseInsensitive))
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
            if(!tmpFileInfo->dir().dirName().compare("win10",Qt::CaseInsensitive)
                    && OSinfo.toString().compare("6.4",Qt::CaseInsensitive))
            {
                passedDirs += tmpFileInfo->dir().path();
                continue;
            }
        }
        filesInCurrentDir.clear();
        filesInCurrentDir = tmpFileInfo->dir().entryList(indicators, QDir::Files);
        if (AppSettings->value("DebugMode").toBool())
        {
            QString body = "Parsing dir " + tmpFileInfo->dir().absolutePath() + " with indicators:\n";
            foreach(QString s,filesInCurrentDir)
            {
                QTextStream(&body) << s << "\n";
            }
            SDDebugMessage("Scaner::run() dir().entryList(indicators, QDir::Files)",body);
        }
        if (filesInCurrentDir.size() &&
                //иногда фильтр не срабатывает отдаёт все файлы :(
                (filesInCurrentDir.contains("info.txt",Qt::CaseInsensitive) || filesInCurrentDir.contains("auto.bat",Qt::CaseInsensitive) || filesInCurrentDir.contains("auto.cmd",Qt::CaseInsensitive)))
        {//есть индикаторы
            if (AppSettings->value("UseInfo.txt").toBool()
                    && (filesInCurrentDir.indexOf("info.txt")!=-1))
            {//есть info.txt
                if (AppSettings->value("Info.txtIsPrefer").toBool())
                {//info.txt приоритетнее
                    if(AppSettings->value("IgnoreDuplicate").toBool())
                    {//игнорим батники
                        tmpAppInfo=parseInfoTxt(tmpFileInfo->absoluteDir().absolutePath()+QDir::separator()+"info.txt",tmpAppInfo);
                        if (!tmpAppInfo.isChecked) emit app(tmpAppInfo);
                        passedDirs += tmpFileInfo->dir().absolutePath();
                        continue;
                    }else
                    {//не игнорим батники но перезаписываем их инфой из info.txt
                        if (AppSettings->value("UseAuto.bat").toBool()
                                && (filesInCurrentDir.indexOf("auto.bat")!=-1))
                        {
                            tmpAppInfo=parseBatchFile(tmpFileInfo->absoluteDir().absolutePath()+QDir::separator()+"auto.bat",tmpAppInfo);
                        }
                        if (AppSettings->value("UseAuto.cmd").toBool()
                                && (filesInCurrentDir.indexOf("auto.cmd")!=-1))
                        {
                            tmpAppInfo=parseBatchFile(tmpFileInfo->absoluteDir().absolutePath()+QDir::separator()+"auto.cmd",tmpAppInfo);
                        }
                        tmpAppInfo=parseInfoTxt(tmpFileInfo->absoluteDir().absolutePath()+QDir::separator()+"info.txt",tmpAppInfo);
                        if (!tmpAppInfo.isChecked) emit app(tmpAppInfo);
                        passedDirs += tmpFileInfo->dir().absolutePath();
                        continue;
                    }
                }else
                {//батник приоритетнее
                    tmpAppInfo=parseInfoTxt(tmpFileInfo->absoluteDir().absolutePath()+QDir::separator()+"info.txt",tmpAppInfo);
                    if (tmpAppInfo.isChecked)
                    {
                        passedDirs += tmpFileInfo->dir().absolutePath();
                        continue;
                    }
                    //взяли инфу из info.txt
                    if (AppSettings->value("UseAuto.bat").toBool()
                            && (filesInCurrentDir.indexOf("auto.bat")!=-1))
                    {//нашли батник, переписали ним
                        tmpAppInfo=parseBatchFile(tmpFileInfo->absoluteDir().absolutePath()+QDir::separator()+"auto.bat",tmpAppInfo);
                        emit app(tmpAppInfo);
                        passedDirs += tmpFileInfo->dir().absolutePath();
                        continue;
                    }
                    if (AppSettings->value("UseAuto.cmd").toBool()
                            && (filesInCurrentDir.indexOf("auto.cmd")!=-1))
                    {//не нашли батник, но нашли cmd, переписали ним
                        tmpAppInfo=parseBatchFile(tmpFileInfo->absoluteDir().absolutePath()+QDir::separator()+"auto.cmd",tmpAppInfo);
                        emit app(tmpAppInfo);
                        passedDirs += tmpFileInfo->dir().absolutePath();
                        continue;
                    }
                    //ничо не нашли, отправили как есть
                    emit app(tmpAppInfo);
                    passedDirs += tmpFileInfo->dir().absolutePath();
                    continue;
                }
            }else
            {//нет info.txt
                if(AppSettings->value("IngnoreBathOnly").toBool())
                {
                    if (!tmpFileInfo->dir().entryList(apps, QDir::Files).size())
                    {
                        passedDirs += tmpFileInfo->dir().absolutePath();
                        continue;
                    }
                }
                if (AppSettings->value("UseAuto.bat").toBool()
                        && (filesInCurrentDir.indexOf("auto.bat")!=-1))
                {
                    tmpAppInfo=parseBatchFile(tmpFileInfo->absoluteDir().absolutePath()+QDir::separator()+"auto.bat",tmpAppInfo);
                    if(AppSettings->value("IgnoreDuplicate").toBool())
                    {
                        emit app(tmpAppInfo);
                        passedDirs += tmpFileInfo->dir().absolutePath();
                        continue;
                    }
                }
                if (AppSettings->value("UseAuto.cmd").toBool()
                        && (filesInCurrentDir.indexOf("auto.cmd")!=-1))
                {
                    tmpAppInfo=parseBatchFile(tmpFileInfo->absoluteDir().absolutePath()+QDir::separator()+"auto.cmd",tmpAppInfo);
                    if(AppSettings->value("IgnoreDuplicate").toBool())
                    {
                        emit app(tmpAppInfo);
                        passedDirs += tmpFileInfo->dir().absolutePath();
                        continue;
                    }
                }
            }
            //наличие флагов отработали, если попали сюда то что-то пошло не так!
            SDDebugMessage(QString::fromUtf8("Ошибка!"),
                          QString::fromUtf8("Coder id Invalid"
                                            "! \n %1").arg(file),
                          true,
                          iconerror);
        }
        else
        {//нет индикаторов
            tmpAppInfo.dir = tmpFileInfo->dir().absolutePath();
            tmpAppInfo.name = tmpFileInfo->dir().dirName();
            if (AppSettings->value("UseIconsFromExe").toBool()) tmpAppInfo.iconString = file;
            tmpAppInfo.description = tmpFileInfo->fileName();
            tmpAppInfo.isChecked = true;
            tmpAppInfo.ver = GetVer(tmpFileInfo->absoluteFilePath().replace("/","\\"));
            if (tmpAppInfo.ver.contains("ERROR")) tmpAppInfo.ver.clear();
            tmpAppInfo.commands.push_back(file.replace("/","\\"));
            passedDirs += tmpFileInfo->dir().path();
            if (tmpAppInfo.name.isEmpty()) tmpAppInfo.name = tmpFileInfo->fileName();
            //qDebug() << file;
            emit app(tmpAppInfo);
            continue;
        }
    }
    while (semaphore) sleep(1);
}

QStringList Scaner::loadFiles(QDir startDir, QStringList filters, bool rec)
{
    if (!rec) nestIndex = 1;
    QStringList list;
    foreach (QString file, startDir.entryList(filters, QDir::Files))
        list += QFileInfo(startDir.absolutePath() + QDir::separator() + file).absoluteFilePath();
    if (AppSettings->value("RecursiveSearch").toBool() || nestIndex)
    {
        foreach (QString subdir, startDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
            list += loadFiles(QDir(startDir.absolutePath() + QDir::separator() + subdir), filters,1);
        if (rec && nestIndex) nestIndex--;
    }
    return list;
}

QStringList Scaner::getTree(QDir startDir)
{
    QStringList list;
    foreach (QString folder, startDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        list += QFileInfo(startDir.absolutePath() + QDir::separator() + folder).absoluteFilePath();
    foreach (QString subdir, startDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot))
        list += getTree(QDir(startDir.absolutePath() + QDir::separator() + subdir));
    return list;
}

QString Scaner::codecDetect(QString file)
{
    QString codec;
    QStringList unsupportedCodecs;
    unsupportedCodecs += "UTF-7";
    unsupportedCodecs += "SCSU";
    unsupportedCodecs += "UTF-8-EBCDIC";
    unsupportedCodecs += "UTF-16LE";
    unsupportedCodecs += "UTF-16BE";
    unsupportedCodecs += "UTF-32LE";
    unsupportedCodecs += "UTF-32BE";
    unsupportedCodecs += "UTF-1";
    unsupportedCodecs += "GB-18030";
    unsupportedCodecs += "BOCU-1";
    codec = "Windows-1251";
    QFile fl(file);
    if (!fl.open(QFile::ReadOnly))
    {
        SDDebugMessage(QString("Scaner::codecDetect(%1,appInfo current)").arg(file),
                      QString("Can not open file %1").arg(file));
        return codec;
    }
    QByteArray bom = fl.read(4);
    if ( bom.contains("+/v8")) codec = "UTF-7";
    if ( bom.contains("+/v9")) codec = "UTF-7";
    if ( bom.contains("+/v+")) codec = "UTF-7";
    if ( bom.contains("+/v/")) codec = "UTF-7";
    //if ( bom.contains("\x00\x00\xFE\xFF")) codec = "UTF-32BE";
    //if ( bom.contains("\xFF\xFE\x00\x00")) codec = "UTF-32LE";
    if ( bom.contains("\335\163\146\163")) codec = "UTF-8-EBCDIC";
    if ( bom.contains("\xDD\x73\x66\x73")) codec = "UTF-8-EBCDIC";
    if ( bom.contains("\x84\x31\x95\x33")) codec = "GB-18030";
    if ( bom.contains("\xEF\xBB\xBF")) codec = "UTF-8";
    if ( bom.contains("\xF7\x64\x4C")) codec = "UTF-1";
    if ( bom.contains("\016\376\377")) codec = "SCSU";
    if ( bom.contains("\xFB\xEE\x28")) codec = "BOCU-1";
    if ( bom.contains("\376\377"))codec = "UTF-16LE";
    if ( bom.contains("\377\376"))codec = "UTF-16BE";
    if (unsupportedCodecs.contains(codec))
    {
        SDDebugMessage(QString("Scaner::codecDetect(%1,appInfo current)").arg(file.replace("/","\\")),QString("Detect unsupported codec %1").arg(codec),true,iconerror);
        codec = "Windows-1251";
    }
    fl.close();
    return codec;

}

appInfo Scaner::parseInfoTxt(QString file,appInfo current)
{
    QSettings infoTxt(file,QSettings::IniFormat);
    QString codec = codecDetect(file);
    SDDebugMessage(QString("Scaner::parseInfoTxt(%1)").arg(file.replace("/","\\")),QString("Detect codec %1").arg(codec));
    infoTxt.setIniCodec(QTextCodec::codecForName(codec.toAscii()));
    infoTxt.allKeys();
    current.dir = QFileInfo(file).absolutePath();
    if (infoTxt.contains("IgnoreOS"))
    {
        QString os = OSinfo.toString("%Name%");
        QString arc = OSinfo.toString("%Arch%");
        QString arcOs = OSinfo.toString("%Arch%.%Name%");
        foreach (QString tmp,infoTxt.value("IgnoreOS").toStringList())
        {
            if (tmp == os || tmp == arc || tmp == arcOs)
            {
                current.isChecked = true;
                return current;
            }
        }
    }
    if (infoTxt.contains("Key"))
    {
        current.key = infoTxt.value("Key").toString();
    }
    if (infoTxt.contains("Kits"))
    {
        current.kits = infoTxt.value("Kits").toStringList();
    }
    if (infoTxt.contains("Depends"))
    {
        current.depends = infoTxt.value("Depends").toStringList();
    }
    if (infoTxt.contains("Conflicts"))
    {
        current.conflicts = infoTxt.value("Conflicts").toStringList();
    }
    if (infoTxt.contains("Addons"))
    {
        current.addons = infoTxt.value("Addons").toStringList();
    }
    //execute
    if (infoTxt.contains("Execute"))
    {
        current.commands += infoTxt.value("Execute").toStringList();
    }
    //
    if (infoTxt.contains(OSinfo.toString("Execute.%Arch%")))
    {
        current.commands += infoTxt.value(OSinfo.toString("Execute.%Arch%")).toStringList();
    }
    //
    if (OSinfo.MajorVersion < 6)
    {
        if (infoTxt.contains(OSinfo.toString("Execute.XP_family")))
        {
            current.commands += infoTxt.value(OSinfo.toString("Execute.XP_family")).toStringList();
        }
        if (infoTxt.contains(OSinfo.toString("Execute.%Arch%.XP_family")))
        {
            current.commands += infoTxt.value(OSinfo.toString("Execute.%Arch%.XP_family")).toStringList();
        }
    }else
    {
        if (infoTxt.contains(OSinfo.toString("Execute.Vista_family")))
        {
            current.commands += infoTxt.value(OSinfo.toString("Execute.Vista_family")).toStringList();
        }
        if (infoTxt.contains(OSinfo.toString("Execute.%Arch%.Vista_family")))
        {
            current.commands += infoTxt.value(OSinfo.toString("Execute.%Arch%.Vista_family")).toStringList();
        }
    }
    //
    if (infoTxt.contains(OSinfo.toString("Execute.%Arch%.%Name%")))
    {
        current.commands += infoTxt.value(OSinfo.toString("Execute.%Arch%.%Name%")).toStringList();
    }
    //
    if (infoTxt.contains("Execute.common"))
    {
        current.commands += infoTxt.value("Execute.common").toStringList();
    }
    //end execute

    if (infoTxt.contains("Icon"))
    {
        QFileInfo iconFileInfo(infoTxt.value("Icon").toString());
        if (!iconFileInfo.exists())
        {
            iconFileInfo.setFile(QFileInfo(file).absolutePath()+QDir::separator()+infoTxt.value("Icon").toString());
        }
        if (!iconFileInfo.exists())
        {
            iconFileInfo.setFile(ExpandEnvironmentString(infoTxt.value("Icon").toString()));
        }
        if (!iconFileInfo.exists())
        {
            iconFileInfo.setFile(QFileInfo(file).absolutePath()+QDir::separator()+ExpandEnvironmentString(infoTxt.value("Icon").toString()));
        }
        if (iconFileInfo.exists())
        {
            if (iconFileInfo.isExecutable())
            {
                if(AppSettings->value("UseIconsFromExe").toBool())
                {
                    current.iconString = iconFileInfo.absoluteFilePath();
                }/*else{
                    current.iconString = infoTxt.value("Icon").toString();
                }*/
            }else
            {
                current.iconString = iconFileInfo.absoluteFilePath();
            }
        }
//        else current.iconString.clear();
    }
    if (infoTxt.contains("Name"))
    {
        current.name = infoTxt.value("Name").toString();
    }
    if (infoTxt.contains("Description"))
    {
        current.description = infoTxt.value("Description").toString();
    }
    if (infoTxt.contains("FullDescription"))
    {
        current.fullDescription = infoTxt.value("FullDescription").toString();
    }
    if (infoTxt.contains("Ver"))
    {
        current.ver = infoTxt.value("Ver").toString();
        if (!current.ver.indexOf("FROMFILE:"))
        {
            QStringRef vfile(&current.ver, 9, current.ver.size()-9);
            if (QFileInfo(vfile.toString()).isAbsolute())
            {
                SDDebugMessage(QString::fromUtf8("Внимание!"),
                              QString::fromUtf8("Пожалуйста не используйте абсолютные пути в файлах info.txt \n"
                                                "Это приведет к сбоям в случае изменения дислокации или работе через сеть \n"
                                                "Ошибка при разборе файла %1 \n"
                                                "Значение %2  \n").arg(file.replace("/","\\")).arg(vfile.toString()),
                              true,
                              iconwarning);
                current.ver = GetVer(ExpandEnvironmentString(vfile.toString()));
            }else
            {
                if(QFileInfo(QFileInfo(file).absolutePath()+QDir::separator()+vfile.toString()).exists())
                {
                    current.ver = GetVer(QFileInfo(file).absolutePath()+QDir::separator()+vfile.toString());
                }else if(QFileInfo(ExpandEnvironmentString(vfile.toString())).exists())
                {
                    current.ver = GetVer(ExpandEnvironmentString(vfile.toString()));
                }

            }
        }
    }
    if (infoTxt.contains("VerCheck"))
    {
        current.instVer = infoTxt.value("VerCheck").toString();
        current.instVer.replace("%directory%",current.dir);
        current.instVer.replace("%name%",current.name);
        current.instVer.replace("%key%",current.key);
    }
    if (infoTxt.contains("Url"))
    {
        current.url = infoTxt.value("Url").toString();
    }
    if (infoTxt.contains("License"))
    {
        current.license = infoTxt.value("License").toString();
    }
    if (infoTxt.contains("KillTimer"))
    {
        current.killTimer = infoTxt.value("KillTimer").toInt();
    }else
    {
        current.killTimer = 3600;
    }
    if (current.fullDescription.isEmpty()) current.fullDescription = current.description;
    return current;
}

appInfo Scaner::parseBatchFile(QString file,appInfo current)
{
    current.dir = QFileInfo(file).absolutePath();
    QStringList appsInDir = QFileInfo(file).absoluteDir().entryList(apps, QDir::Files);
    current.name = QFileInfo(file).absoluteDir().dirName();
    if (appsInDir.size())
    {//есть проги
        current.description = QFileInfo(appsInDir[0]).fileName();
        current.ver = GetVer(QFileInfo(appsInDir[0]).absoluteFilePath().replace("/","\\"));
        if (current.ver.contains("ERROR")) current.ver.clear();
        if (AppSettings->value("UseIconsFromExe").toBool())
            current.iconString = QFileInfo(file).absolutePath()+QDir::separator()+appsInDir[0];
        else
            current.iconString = file;
    }
    else
    {//только батник
        if (AppSettings->value("IngnoreBathOnly").toBool())
        {//мы сюда никогда не должны попасть не?
            SDDebugMessage(QString::fromUtf8("Ошибка!"),
                          QString::fromUtf8("Попытка распарсить группу программ %1 как приложение!\nCoder id Invalid"
                                            "!").arg(QFileInfo(file).absoluteDir().absolutePath()),
                          true,
                          iconerror);
            passedDirs += QFileInfo(file).absoluteDir().absolutePath();
            return current;
        }
        //current.description = QFileInfo(file).fileName();
        current.description =  current.name;
        current.iconString = file;
        current.kits.push_back("kit");
    }
    current.commands.push_back(file.replace("/","\\"));
    if (current.fullDescription.isEmpty() && !current.description.isEmpty())
        current.fullDescription = current.description;
    return current;
}

