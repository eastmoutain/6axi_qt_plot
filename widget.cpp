#include "widget.h"
#include "ui_widget.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <QString>
#include <QDebug>
#include <QMessageBox>
#include <QHostAddress>
#include <QtWidgets/QBoxLayout>
#include <QTime>

#define X_POINT_SIZE 200

void normalize(double v[3])
{
    double n = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);

    v[0] = v[0] / n;
    v[1] = v[1] / n;
    v[2] = v[2] / n;

}

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    isConnected(false),
    gyro_calibrated(false),
    acc_est_initialized(false)
{
    ui->setupUi(this);

    gyro_cali.gx = gyro_cali.gy = gyro_cali.gz = 0.000000001;
    init_plot();

    ui->verticalLayout_2->addWidget(plot);

    ax = new QVector<double>;
    ay = new QVector<double>;
    az = new QVector<double>;
    eax = new QVector<double>;
    eay = new QVector<double>;
    eaz = new QVector<double>;
    at = new QVector<double>;

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(connected()), this, SLOT(connectedSlot()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
    connect(socket, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(errorSlot(QAbstractSocket::SocketError)));





}

void Widget::init_plot()
{
    plot = new QwtPlot(this); //creation of the plot
    plot->setMinimumSize(700,450); //we set the minimum size of the plot

    plot->setTitle("T = f(t)"); //title of the graphe

    // axes names
    plot->setAxisScale( QwtPlot::xBottom, 0, X_POINT_SIZE);
    plot->setAxisTitle(QwtPlot::xBottom, "t (s)" );
    plot->setAxisScale( QwtPlot::yLeft, -1.0, 1.0 );
    plot->setAxisTitle(QwtPlot::yLeft, "y (g)");

    plot->setLineWidth(1);
    plot->setFrameStyle(QFrame::Box | QFrame::Plain);

    // creation of the curve (you can add more curve to a graphe)
    curve_ax = new QwtPlotCurve;
    curve_ax->setPen(QPen(Qt::red));
    curve_ax->attach(plot);

    curve_ay = new QwtPlotCurve;
    curve_ay->setPen(QPen(Qt::blue));
    curve_ay->attach(plot);

    curve_az = new QwtPlotCurve;
    curve_az->setPen(QPen(Qt::green));
    curve_az->attach(plot);

    curve_eax = new QwtPlotCurve;
    curve_eax->setPen(QPen(Qt::gray));
    curve_eax->attach(plot);

    curve_eay = new QwtPlotCurve;
    curve_eay->setPen(QPen(Qt::yellow));
    curve_eay->attach(plot);

    curve_eaz = new QwtPlotCurve;
    curve_eaz->setPen(QPen(Qt::black));
    curve_eaz->attach(plot);
}

Widget::~Widget()
{
    delete socket;
    delete ui;
}

void Widget::connectedSlot()
{
    isConnected = true;
    g_timer.start();
}

void Widget::disconnectedSlot()
{
    isConnected = false;
    socket->close();
}

void Widget::readyReadSlot()
{
    static int i = 0;
    struct gyro_data _gyro;
    struct acc_data _acc;
    const double degree_rad = 3.1415926 / 180;

    //QByteArray message;
    //message = socket->readAll();
    int n = socket->read((char *)&sd_raw, sizeof(struct sensor_data));

    if (!gyro_calibrated) {
        gyro_cali.gx += sd_raw.gx;
        gyro_cali.gy += sd_raw.gy;
        gyro_cali.gz += sd_raw.gz;
        i++;
        if (i == 50) {
            gyro_cali.gx = (gyro_cali.gx) /50;
            gyro_cali.gy = (gyro_cali.gy) /50;
            gyro_cali.gz = (gyro_cali.gz) /50;
            qDebug() << "gyro_cali.gx  " << gyro_cali.gx
                     << ", gyro_cali.gy " << gyro_cali.gy
                     << ", gyro_cali.gz " << gyro_cali.gz;

            gyro_calibrated = true;
        } else {
            return;
        }

    }

    int timer_diff = g_timer.elapsed() * 0.001;
    g_timer.start();


    sd.ax = -sd_raw.ax;
    sd.ay = -sd_raw.ay;
    sd.az = -sd_raw.az;
    qDebug() << "sd.ax " << sd.ax << ", sd.ay " << sd.ay << ", sd.az " << sd.az;
    _acc.ax = (double)sd.ax / 16384;
    _acc.ay = (double)sd.ay / 16384;
    _acc.az = (double)sd.az / 16384;

    //qDebug() << "n " << n << "ax " << _acc.ax << ", ay " << _acc.ay << ", az " << _acc.az
    //         << "gx" << sd.gx << ", gy " << sd.gy << ", gz " << sd.gz;

    normalize(_acc.a);

    //addpoint(_acc.ax, _acc.ay, _acc.az);
    qDebug() << "_acc.ax " << _acc.ax << ", _acc.ay " << _acc.ay << ", _acc.az " << _acc.az;


    if (!acc_est_initialized) {
        Rest_pre[X] = _acc.ax;
        Rest_pre[Y] = _acc.ay;
        Rest_pre[Z] = _acc.az;
        acc_est_initialized = true;
    }


    sd.gx = -(sd_raw.gx - gyro_cali.gx);
    sd.gy = -(sd_raw.gy - gyro_cali.gy);
    sd.gz = -(sd_raw.gz - gyro_cali.gz);
    _gyro.gx = (double)sd.gx / 131;
    _gyro.gy = (double)sd.gy / 131;
    _gyro.gz = (double)sd.gz / 131;
    qDebug() << "_gyro.gx " << _gyro.gx << ", _gyro.gy " << _gyro.gy << ", _gyro.gz " << _gyro.gz;


    normalize(_gyro.g);

    // atan2 return rad
    Axz_pre = atan2(Rest_pre[X], Rest_pre[Z]);
    Ayz_pre = atan2(Rest_pre[Y], Rest_pre[Z]);

    Axz_now = Axz_pre + _gyro.gy * timer_diff * degree_rad;
    Ayz_now = Ayz_pre + _gyro.gx * timer_diff * degree_rad;

    Rgyro[X] = sin(Axz_now) / sqrt(1 + cos(Axz_now) * cos(Axz_now) * tan(Ayz_now) * tan(Ayz_now));
    Rgyro[Y] = sin(Ayz_now) / sqrt(1 + cos(Ayz_now) * cos(Ayz_now) * tan(Axz_now) * tan(Axz_now));

    int sign;
    if (Rest_pre[Z] >= 0.000000000000001) sign = 1;
    else sign = -1;

    Rgyro[Z] = sign * sqrt(1- Rgyro[X] * Rgyro[X] - Rgyro[Y] * Rgyro[Y]);

    qDebug() << "Rest_pre[X] " << Rest_pre[X] << ", Rest_pre[Y] " << Rest_pre[Y] << ", Rest_pre[Z] " << Rest_pre[Z];

    if (Rest_pre[Z] > -0.00001 && Rest_pre[Z] < 0.00001) {
        qDebug() << "################################################ Rest_pre[Z] < 0.00001 ";
        Rgyro[X] = Rest_pre[X];
        Rgyro[Y] = Rest_pre[Y];
        Rgyro[Z] = Rest_pre[Z];
    }

    double rate = 0.6;
    Rest_pre[X] = (_acc.ax + Rgyro[X] * rate) / (1 + rate);
    Rest_pre[Y] = (_acc.ay + Rgyro[Y] * rate) / (1 + rate);
    Rest_pre[Z] = (_acc.az + Rgyro[Z] * rate) / (1 + rate);
    normalize(Rest_pre);

    addpoint(Rest_pre[X], Rest_pre[Y], Rest_pre[Z], _acc.ax, _acc.ay, _acc.az);
    qDebug() << "Rgyro[X] " << Rgyro[X] << ", Rgyro[Y] " << Rgyro[Y] << ", Rgyro[Z] " << Rgyro[Z];



}

void Widget::errorSlot(QAbstractSocket::SocketError)
{
    QMessageBox::information(this, "show", socket->errorString());
    disconnectedSlot();
}

void Widget::on_pushButton_clicked()
{
    QString str = ui->addr_lineEdit->text();
    int port = ui->port_lineEdit_2->text().toInt();
    qDebug() << "addr: " << str << "port" << port << "\r\n";
    socket->connectToHost(str,  port);
}

void Widget::addpoint(double _ax, double _ay, double _az, double _eax, double _eay, double _eaz)
{
    if (at->size() == X_POINT_SIZE) {
        ax->pop_back();
        ax->push_front(_ax);
        ay->pop_back();
        ay->push_front(_ay);
        az->pop_back();
        az->push_front(_az);
        eax->pop_back();
        eax->push_front(_eax);
        eay->pop_back();
        eay->push_front(_eay);
        eaz->pop_back();
        eaz->push_front(_eaz);
    } else {
        at->push_front(at->size());
        ax->push_front(_ax);
        ay->push_front(_ay);
        az->push_front(_az);
        eax->push_front(_eax);
        eay->push_front(_eay);
        eaz->push_front(_eaz);
    }
    //qDebug() << "data_x.size = " << data_x->size() << "data_y.size()" << data_y->size() << "  " << point;

    curve_ax->setSamples(*at, *ax);// we set the data to the curve
    curve_ay->setSamples(*at, *ay);
    curve_az->setSamples(*at, *az);
    curve_eax->setSamples(*at, *eax);
    curve_eay->setSamples(*at, *eay);
    curve_eaz->setSamples(*at, *eaz);

    plot->replot(); // we redraw the graphe
}

void Widget::on_pushButton_2_clicked()
{
    disconnectedSlot();
}
