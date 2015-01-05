#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>

namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT
public:
    explicit Settings(QWidget *parent = 0);
    ~Settings();
signals:
    void showInfo(bool show);
private slots:
    void on_exitBtn_clicked();
    void on_pushButton_clicked();
    void on_debugBox_clicked(bool checked);
    void on_consoleBox_clicked(bool checked);
private:
    Ui::Settings *ui;
    void init();
};

#endif // SETTINGS_H
