#ifndef SDVERSION_H
#define SDVERSION_H

#include <QString>

class SdVersion
{
public:
    SdVersion();
    SdVersion(int major, int minor, int build, int revision);
    SdVersion(QString v);
    const bool operator>(SdVersion v);
    const bool operator<(SdVersion v);
    const bool operator==(SdVersion v);
    const bool operator!=(SdVersion v){return !operator==(v);}
private:
    int major;
    int minor;
    int build;
    int revision;
};

#endif // SDVERSION_H
