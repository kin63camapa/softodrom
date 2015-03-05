#ifndef INSTALLER_H
#define INSTALLER_H
typedef void *HANDLE;

#include <QThread>
#include <QTimer>
#include "appbox.h"


class Installer : public QThread
{
    Q_OBJECT
public:
    explicit Installer(QObject *parent = 0);
    void setApps(QList<appBox*> apps);
protected:
    void run();
signals:
    void result(appBox*,appBox::STATE);
private:
    QList<appBox*> list;
    HANDLE SDRunExternalEx(QString cmd, QString dir,appBox *box);
    appBox::STATE status;
};

#endif // INSTALLER_H
