#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QResizeEvent>
#include "appbox.h"
#include "scaner.h"
#include "settings.h"
#include "kitmenu.h"
#include "installer.h"
#include "aboutbox.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private slots:
    void showInfo(bool show);
    void showSettings();
    void rebulildBoxes();
    void rescanApps();
    void scanComplete();
    void initSoftProgBar(int size);
    void incSoftProgBar();
    void initUpdProgBar(int size);
    void incUpdProgBar();
    void on_renewButton_clicked();
    void newApp(appInfo app);
    //void installCheked();
    void installOne(appBox*app);
    void on_startStopButton_clicked();
    void enableKit(QString kitname);
    void disableKit(QString kitname);
    void markedKit(QString kitname);
    void unmarkedKit(QString kitname);
    void uncheckInstalled();
    void on_markedAllAction_triggered();
    void on_unmarkedAllAction_triggered();
    void on_markedSoftOnly_triggered();
    void on_markedUpdatesOnly_triggered();
    void on_hideButton_clicked();
    void on_showButton_clicked();
    void result(appBox*,appBox::STATE status);
//    void start(appBox* box);
    void installComplete();
    void showAll();
    void hideAll();
    void on_actionHelp_triggered();
    void on_actionAbout_triggered();
    void on_niniteOpenBtn_clicked();
    void avirInstalledSlot();
    void on_drpOpenBtn_clicked();
    void avirChecked(appBox*app);
    void conflictsCheck(QStringList conflicts, QString name);
    void dependsNeed(QStringList depends);
    void sortSoft();
private:
    AboutBox *aboutBox;
    QAction *showAllAction;
    QAction *hideAllAction;
    QStringList kits;
    Scaner * scaner;
    Installer * installer;
    QList<KitMenu*> kitsMenus;
    QList<appBox*> soft;   
    QList<appBox*> update;
    int col;
    Ui::MainWindow *ui;
    Settings * settings;
    bool installOneNow;
    bool autoSelectorWorking;
    bool avirInstalled;
protected:
   virtual void resizeEvent(QResizeEvent *event);
   virtual void closeEvent(QCloseEvent *e);
};

#endif // MAINWINDOW_H
