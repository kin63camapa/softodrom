#ifndef SDVERSION_H
#define SDVERSION_H

#include <QString>

class SdVersion
{
public:
    SdVersion();
    SdVersion(int major, int minor, int build, int revision);
    SdVersion(QString v);
    QString toString();
    bool operator>(SdVersion v);
    bool operator<(SdVersion v);
    bool operator==(SdVersion v);
    bool operator!=(SdVersion v);
    bool isValid();
private:
    int major;
    int minor;
    int build;
    int revision;
};

#endif // SDVERSION_H
