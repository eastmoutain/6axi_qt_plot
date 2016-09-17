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

#define X_POINT_SIZE 200

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    isConnected(false)
{
    ui->setupUi(this);

    init_plot();

    ui->verticalLayout_2->addWidget(plot);

    ax = new QVector<double>;
    ay = new QVector<double>;
    az = new QVector<double>;
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
    plot->setAxisScale( QwtPlot::yLeft, -1.5, 1.5 );
    plot->setAxisTitle(QwtPlot::yLeft, "y (g)");

    plot->setLineWidth(1);
    plot->setFrameStyle(QFrame::Box | QFrame::Plain);

    // creation of the curve (you can add more curve to a graphe)
    curve_ax = new QwtPlotCurve;
    curve_ax->setPen(QPen(Qt::red));
    curve_ax->attach(plot);

    curve_ay = new QwtPlotCurve;
    curve_ay->setPen(QPen(Qt::black));
    curve_ay->attach(plot);

    curve_az = new QwtPlotCurve;
    curve_az->setPen(QPen(Qt::green));
    curve_az->attach(plot);
}

Widget::~Widget()
{
    delete socket;
    delete ui;
}

void Widget::connectedSlot()
{
    isConnected = true;
}

void Widget::disconnectedSlot()
{
    isConnected = false;
    socket->close();
}

void Widget::readyReadSlot()
{
    //QByteArray message;
    //message = socket->readAll();
    socket->read((char *)&sd, sizeof(sd));

    //memset(&sd, (int)&message, sizeof(sd));
    qDebug("read ax %d, ay %d, az %d, gx %d, gy %d, gz %d\r\n",
           sd.ax, sd.ay, sd.az, sd.gx, sd.gy, sd.gz);

    addpoint((double)sd.ax / 12768, (double)sd.ay / 12768, (double)sd.az / 12768);

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

void Widget::addpoint(double _ax, double _ay, double _az)
{
    if (at->size() == X_POINT_SIZE) {
        ax->pop_back();
        ax->push_front(_ax);
        ay->pop_back();
        ay->push_front(_ay);
        az->pop_back();
        az->push_front(_az);
    } else {
        at->push_front(at->size());
        ax->push_front(_ax);
        ay->push_front(_ay);
        az->push_front(_az);
    }
    //qDebug() << "data_x.size = " << data_x->size() << "data_y.size()" << data_y->size() << "  " << point;

    curve_ax->setSamples(*at, *ax);// we set the data to the curve
    curve_ay->setSamples(*at, *ay);
    curve_az->setSamples(*at, *az);

    plot->replot(); // we redraw the graphe
}
