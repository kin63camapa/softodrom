#include <QStringList>
#include "sdversion.h"

SdVersion::SdVersion()
{
    major=-10;
    minor=-10;
    build=-10;
    revision=-10;
}

SdVersion::SdVersion(int major, int minor, int build, int revision)
{
    this->major=major;
    this->minor=minor;
    this->build=build;
    this->revision=revision;
}


SdVersion::SdVersion(QString v)
{
    major=-10;
    minor=-10;
    build=-10;
    revision=-10;
    QStringList list = v.split(QRegExp("\\."), QString::SkipEmptyParts);
    if (list.size() < 0 | list.size() > 4)
    {
        return;
    }else
    {
        major=list[0].toInt();
        if(list.size()>1)minor=list[1].toInt();
        if(list.size()>2)build=list[2].toInt();
        if(list.size()>3)revision=list[3].toInt();
    }
}

QString SdVersion::toString()
{
    if (revision < 0 & build < 0 & minor < 0 & major < 0) return "";
    if (revision < 0 & build < 0 & minor < 0) return QString("%1").arg(major);
    if (revision < 0 & build < 0) return QString("%1.%2").arg(major).arg(minor);
    if (revision < 0) return QString("%1.%2.%3").arg(major).arg(minor).arg(build);
    return QString("%1.%2.%3.%4").arg(major).arg(minor).arg(build).arg(revision);
}

bool SdVersion::operator>(SdVersion v)
{
    if (major>v.major) return true;
    if (major<v.major) return false;
    if (minor>v.minor) return true;
    if (minor<v.minor) return false;
    if (build>v.build) return true;
    if (build<v.build) return false;
    if (revision>v.revision) return true;
    if (revision<v.revision) return false;
    return false;
}

bool SdVersion::operator<(SdVersion v)
{
    if (major<v.major) return true;
    if (major>v.major) return false;
    if (minor<v.minor) return true;
    if (minor>v.minor) return false;
    if (build<v.build) return true;
    if (build>v.build) return false;
    if (revision<v.revision) return true;
    if (revision>v.revision) return false;
    return false;
}

bool SdVersion::operator==(SdVersion v)
{
    if (major<0) return false;
    if (major==v.major&minor==v.minor&build==v.build&revision==v.revision) return true;
    return false;
}

bool SdVersion::isValid()
{
    return revision >= 0 & build >= 0 & minor >= 0 & major >= 0;
}

