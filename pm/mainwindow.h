#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

namespace Ui {
class MainWindow;
}

class MyThread : public QThread
{
    Q_OBJECT
signals:
    void sendTime(QString time);

private:
    void run();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void receive();

private:
    int pm1_0, pm2_5, pm10, co2;
    Ui::MainWindow *ui;
    QUdpSocket *receiver;
    void refresh();
    QColor color_table[10];
    MyThread thread;

};

#endif // MAINWINDOW_H
