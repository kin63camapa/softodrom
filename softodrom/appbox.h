#ifndef APPBOX_H
#define APPBOX_H

#include <QWidget>
#include <QMovie>
struct appInfo
{
public:
    QString iconString;
    QString name;
    QString description;
    QString fullDescription;
    bool isChecked;
    bool isAvir;
    bool mkTmp;
    QString ver;
    QString instVer;
    QString url;
    QString license;
    QString key;
    QString dir;
    QStringList kits;
    QStringList depends;
    QStringList addons;
    QStringList commands;
    QStringList conflicts;
    int killTimer;
    void clear();
};

namespace Ui {
class appBox;
}

class appBox : public QWidget
{
    Q_OBJECT
public:
    explicit appBox(QWidget *parent = 0);
    ~appBox();
    void setInfo(appInfo i);
    void setDir(QString dir){info.dir = dir;}
    appInfo getInfo(){return info;}
    enum STATE
    {
        normal = 0,
        error = 1,
        warning = 2,
        ready = 100,
        setup = 3,
        wait = 99
    } state;
    bool isChecked();
    bool isUnchecked(){return !isChecked();}
public slots:
    void startWait();
    void startInstall();
    void stopInstall(STATE st);
    void setChecked(bool check = true);
    void setUnchecked(bool check = true);
    void clearEmptyCommandsAndReverse();
    void clearEmptyCommands();
    void updateMovie();
    void setMessage(QString message);
signals:
    void now(appBox*app);
    void avirChecked(appBox*app);
    void conflictsCheck(QStringList conflicts, QString name);
    void dependsNeed(QStringList depends);
private slots:
    void on_pushButton_clicked();
    void on_NOWbtn_clicked();
    void openUrl(QString url);
private:
    Ui::appBox *ui;
    QMovie *movie;
    appInfo info;
    QPalette pGreen;
    QPalette pOrange;
    QPalette pRed;
    QPalette pDefault;
};

#endif // APPBOX_H
