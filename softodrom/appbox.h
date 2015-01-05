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
    bool isCheked;
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
    appInfo getInfo(){return info;}
    enum STATE
    {
        normal = 0,
        error = 1,
        warning = 3,
        ready = 100,
        wait = 99
    } state;
    bool isMarkered();
    bool isUnmarkered(){return !isMarkered();}
    QString description();
public slots:
    void startInstall();
    void startAnimation();
    void stopInstall(STATE st);
    void setMarkered(bool mark = true);
    void setUnmarkered(bool mark = true);
    void clearEmptyCommandsAndReverse();
    void clearEmptyCommands();
    void updateMovie();
    void setMessage(QString message);
    //void setDescription(QString desc);
    //void setImage(QPixmap image);
    //void setMarkered();
    //void setUnmarkered();
signals:
    void now(appBox*app);
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
