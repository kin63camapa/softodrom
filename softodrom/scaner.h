#ifndef SCANER_H
#define SCANER_H
#include "appbox.h"
#include <QtCore>

class Scaner : public QThread
{
    Q_OBJECT
public:
    explicit Scaner(QObject *parent = 0);
protected:
    void run();
signals:
    void sizeProgBar(int size);
    void progBarPlus();
    void app(appInfo app);
private:
    int nestIndex;
    QStringList filters;
    QStringList apps;
    QStringList indicators;
    QStringList passedDirs;
    QStringList getTree(QDir startDir);
    QStringList loadFiles(QDir startDir, QStringList filters,bool rec = 0);
    appInfo parseInfoTxt(QString file, appInfo current);
    appInfo parseBatchFile(QString file, appInfo current);
    QString codecDetect(QString file);
};

#endif // SCANER_H
