#include "kitmenu.h"
extern QString SDtranslateKit(QString kit);

KitMenu::KitMenu(QWidget *parent) :
    QMenu(parent)
{
     addToMain = new QAction(QString::fromUtf8("Добавить в список"),this);
     setCheked = new QAction(QString::fromUtf8("Проставить галки"),this);
     setUncheked = new QAction(QString::fromUtf8("Снять галки"),this);;
     setHideDisable = new QAction(QString::fromUtf8("Убрать из списка"),this);
     addAction(addToMain);
     addAction(setHideDisable);
     addAction(setCheked);
     addAction(setUncheked);

}

KitMenu::~KitMenu()
{
}

void KitMenu::setKitName(QString name)
{
    this->setTitle(SDtranslateKit(name));
    this->name = name;
    connect(addToMain,SIGNAL(triggered()),this,SLOT(enable()));
    connect(setHideDisable,SIGNAL(triggered()),this,SLOT(disable()));
    connect(setCheked,SIGNAL(triggered()),this,SLOT(setMarked()));
    connect(setUncheked,SIGNAL(triggered()),this,SLOT(setUnmarked()));

}

void KitMenu::enable()
{
    emit enable(name);
}

void KitMenu::disable()
{
    emit disable(name);
}

void KitMenu::setMarked()
{
    emit setMarked(name);
}

void KitMenu::setUnmarked()
{
    emit setUnmarked(name);
}



