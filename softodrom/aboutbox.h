#ifndef ABOUTBOX_H
#define ABOUTBOX_H
#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QFormLayout>
#include <qevent.h>

class ClickedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit ClickedLabel();
    ~ClickedLabel(){}
signals:
    void clicked();
    void doubleClicked();
    void rightClicked();
    void rightDoubleClicked();
    void midClicked();
    void midDoubleClicked();
    void drag(int x,int y);
    void drop();
public slots:
    void moove(int x, int y);
    void setMoovable(Qt::MouseButton bt = Qt::NoButton);
protected:
    void mousePressEvent(QMouseEvent *ev);
    bool eventFilter(QObject *watched, QEvent *event);
private:
    bool mooovable;
    Qt::MouseButton btn;
    Qt::MouseButton movBtn;
    int lastX,lastY;
};



class AboutBox : public QDialog
{
    Q_OBJECT
public:
    explicit AboutBox();
    ~AboutBox();
public slots:
    void f1();
    void f2();
    void f3();
    void textHoverClicked(QString url);
private:
    ClickedLabel * text;
    ClickedLabel * icon;
    ClickedLabel * back;
    ClickedLabel * dummy;
    QPushButton * btn;
    int counter;
    QFormLayout *formLayout;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacerLeft;
    QSpacerItem *horizontalSpacerRight;
};



#endif // ABOUTBOX_H
