#include "softodromglobal.h"
#include "aboutbox.h"
#include <QMouseEvent>
#include <QIcon>
#include <QDesktopServices>

AboutBox::AboutBox()
{
    QString string;
    QTextStream(&string) << "<samp>"
                         << "<pre>applicationName     : " << QCoreApplication::applicationName()
                         << " <br>appHommePage        : " << "<a href=https://github.com/kin63camapa/softodrom>github.com/kin63camapa/softodrom</a>"
                         << " <br>applicationVersion  : " << QCoreApplication::applicationVersion()
                         << " <br>organizationName    : " << QCoreApplication::organizationName()
                         << " <br>organizationDomain  : " << QCoreApplication::organizationDomain()
                         << " <br>applicationPid      : " << QCoreApplication::applicationPid()
                         << " <br>applicationDirPath  : " << QCoreApplication::applicationDirPath().replace("/","\\")
                         << " <br>applicationFilePath : " << QCoreApplication::applicationFilePath().replace("/","\\")
                         << " <br>qtVersionString     : " << QT_VERSION_STR;
    if (AppSettings->isWritable())
        QTextStream(&string) << " <br>configIsWritable    : <span style=\"color:#00ff00;\">TRUE</span>" ;
    else
        QTextStream(&string) << " <br>configIsWritable    : <span style=\"color:#ff0000;\">FALSE</span>" ;
    if (OSinfo.isAdmin)
        QTextStream(&string) << " <br>runAsAdministrator  : <span style=\"color:#00ff00;\">TRUE</span>" ;
    else
        QTextStream(&string) << " <br>runAsAdministrator  : <span style=\"color:#ff0000;\">FALSE</span>" ;
    if (OSinfo.is64)
    {
        if (OSinfo.is64build)
            QTextStream(&string) << " <br>is64Build           : <span style=\"color:#00ff00;\">TRUE</span>" ;
        else
            QTextStream(&string) << " <br>is64Build           : <span style=\"color:#ff0000;\">FALSE</span>" ;
    }else
    {
        if (OSinfo.is64build)
            QTextStream(&string) << " <br>is64Build           : TRUE" ;
        else
            QTextStream(&string) << " <br>is64Build           : FALSE" ;
    }
    QTextStream(&string)<< "<br></pre></samp>";
    resize(0, 0);
    formLayout = new QFormLayout(this);
    horizontalLayout = new QHBoxLayout();
    horizontalSpacerLeft = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    horizontalSpacerRight = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    icon = new ClickedLabel();
    text = new ClickedLabel();
    back = new ClickedLabel();
    connect(text,SIGNAL(linkActivated(QString)),this,SLOT(textHoverClicked(QString)));
    btn = new QPushButton(this);
    connect(btn,SIGNAL(clicked()),this,SLOT(hide()));

    btn->setText(QString::fromUtf8("Угу"));
    text->setText(string);
    icon->setPixmap(QIcon(":/softodrom.ico").pixmap(48,48));f1();
    back->setMinimumWidth(48+20);

    icon->setParent(this);
    text->setParent(this);


    horizontalLayout->addItem(horizontalSpacerLeft);
    horizontalLayout->addWidget(btn);
    horizontalLayout->addItem(horizontalSpacerRight);

    formLayout->setWidget(0, QFormLayout::LabelRole, icon);
    formLayout->setWidget(0, QFormLayout::FieldRole, text);
    formLayout->setWidget(1, QFormLayout::LabelRole, back);
    formLayout->setLayout(1, QFormLayout::FieldRole, horizontalLayout);

    text->setMouseTracking(false);
}

void AboutBox::textHoverClicked(QString url)
{
    QDesktopServices::openUrl(url);
}

AboutBox::~AboutBox()
{

}

ClickedLabel::ClickedLabel()
{
    installEventFilter(this);
    setMouseTracking(false);
    mooovable = false;
    btn = Qt::NoButton;
}

void ClickedLabel::moove(int x, int y)
{
    if (mooovable)
    {
        if (movBtn == btn || movBtn == Qt::NoButton)
        {
            this->setGeometry(geometry().x()+x-lastX,
                              geometry().y()+y-lastX,
                              geometry().width(),
                              geometry().height());
        }
    }
}

void ClickedLabel::setMoovable(Qt::MouseButton bt)
{
    mooovable = true;
    movBtn = bt;
    connect(this,SIGNAL(drag(int,int)),this,SLOT(moove(int,int)));
}

bool ClickedLabel::eventFilter( QObject * watched, QEvent * event)
{
    if (watched == this)
    {
        if (event->type() == QEvent::MouseMove)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            emit drag(mouseEvent->pos().x(),mouseEvent->pos().y());
            qDebug() << "pos:" << mouseEvent->pos().x() << " " << mouseEvent->pos().y() << " btn:" << btn;
        }
        if (event->type() == QEvent::MouseButtonRelease)
        {
            btn = Qt::NoButton;
            emit drop();
        }
        //event->type()
        //10 - получение фокуса
        //11 - потеря фокуса
        //110 - остановка
        //5 - moove
        //3 - drop
    }
    return QLabel::eventFilter(watched,event);
}

void ClickedLabel::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() == Qt::LeftButton)
    {
        if (ev->type() == QEvent::MouseButtonPress)
        {
            lastX = ev->pos().x();
            lastY = ev->pos().y();
            btn = Qt::LeftButton;
            emit clicked();
        }
        if (ev->type() == QEvent::MouseButtonDblClick) emit doubleClicked();
    }
    if (ev->button() == Qt::RightButton)
    {
        if (ev->type() == QEvent::MouseButtonPress)
        {
            lastX = ev->pos().x();
            lastY = ev->pos().y();
            btn = Qt::RightButton;
            emit rightClicked();
        }
        if (ev->type() == QEvent::MouseButtonDblClick) emit rightDoubleClicked();
    }
    if (ev->button() == Qt::MidButton)
    {
        if (ev->type() == QEvent::MouseButtonPress)
        {
            lastX = ev->pos().x();
            lastY = ev->pos().y();
            btn = Qt::MidButton;
            emit midClicked();
        }
        if (ev->type() == QEvent::MouseButtonDblClick) emit midDoubleClicked();
    }
}

