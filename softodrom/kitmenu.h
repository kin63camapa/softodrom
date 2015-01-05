#ifndef KITMENU_H
#define KITMENU_H

#include <QMenu>
#include <QAction>

class KitMenu : public QMenu
{
    Q_OBJECT
public:
    explicit KitMenu(QWidget *parent = 0);
    ~KitMenu();
    void setKitName(QString name);

signals:
    void enable(QString);
    void disable(QString);
    void setMarked(QString);
    void setUnmarked(QString);
public slots:
    void enable();
    void disable();
    void setMarked();
    void setUnmarked();
private:
    QString name;
    QAction* addToMain;
    QAction* setCheked;
    QAction* setUncheked;
    QAction* setHideDisable;
};

#endif // KITMENU_H
