#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <time.h>

MainWindow * thiswindow;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    thiswindow = this;
    ui->setupUi(this);

    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setGeometry(QRect(0,0,1024,600));

    QPalette palette = ui->label->palette();
    palette.setColor(palette.WindowText, QColor(128, 128, 128));
    ui->label->setPalette(palette);

    QFont font = ui->label_time->font();
    font.setPixelSize(48);
    ui->label_time->setFont(font);

    receiver = new QUdpSocket(this);
    receiver->bind(QHostAddress::LocalHost, 20160);
    connect(receiver, SIGNAL(readyRead()), this, SLOT(receive()));
    pm1_0 = 10;
    pm2_5 = 10;
    pm10 = 10;
    co2 = 400;

    color_table[0] = QColor(  0, 228,  0);
    color_table[1] = QColor(255, 255,  0);
    color_table[2] = QColor(225, 126,  0);
    color_table[3] = QColor(225,   0,  0);
    color_table[4] = QColor(153,   0, 76);
    color_table[5] = QColor(126,   0, 35);
    color_table[6] = QColor(128, 255,128);
    refresh();

    QObject::connect(&thread, SIGNAL(sendTime(QString)), ui->label_time, SLOT(setText(QString)), Qt::QueuedConnection);
    thread.start();
}

void MyThread::run(){
    while(true){
        QString newTime = QDateTime::currentDateTime().toString("yyyy-MM-dd\nhh:mm:ss");
        emit sendTime(newTime);
        sleep(1);
    }
}

void MainWindow::refresh()
{
    char buf[128];
    sprintf(buf, "Raw: PM1.0: %d, PM2.5: %d, PM10: %d, CO2: %d\n", pm1_0, pm2_5, pm10, co2);

    ui->label->setText(buf);
    ui->lcdpm1_0->display(pm1_0);
    ui->lcdpm2_5->display(pm2_5);
    ui->lcdpm10->display(pm10);
    ui->lcdco2->display(co2);

    int level = 0;

    if(pm2_5 <=  35)level = 0;
    else if(pm2_5 <=  75)level = 1;
    else if(pm2_5 <= 115)level = 2;
    else if(pm2_5 <= 150)level = 3;
    else if(pm2_5 <= 250)level = 4;
    else level = 5;

    QPalette palette = ui->lcdpm2_5->palette();
    palette.setColor(palette.Background, color_table[level]);
    ui->lcdpm2_5->setPalette(palette);

    if(pm10 <=  50)level = 0;
    else if(pm10 <= 150)level = 1;
    else if(pm10 <= 250)level = 2;
    else if(pm10 <= 350)level = 3;
    else if(pm10 <= 420)level = 4;
    else level = 5;

    palette = ui->lcdpm10->palette();
    palette.setColor(palette.Background, color_table[level]);
    ui->lcdpm10->setPalette(palette);

    if(co2 <=  450)level = 0;
    else if(co2 <= 1000)level = 6;
    else if(co2 <= 2000)level = 1;
    else if(co2 <= 5000)level = 2;
    else level = 3;

    palette = ui->lcdco2->palette();
    palette.setColor(palette.Background, color_table[level]);
    ui->lcdco2->setPalette(palette);

}

void MainWindow::receive()
{
    while (receiver->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(receiver->pendingDatagramSize());
        receiver->readDatagram(datagram.data(), datagram.size());

        sscanf(datagram.data(), "PM1.0: %d, PM2.5: %d, PM10: %d, CO2: %d", &pm1_0, &pm2_5, &pm10, &co2);
        refresh();
    }
}

void MainWindow::on_pushButton_clicked()
{
    exit(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}
