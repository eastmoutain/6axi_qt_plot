#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork/QTcpSocket>
#include <QVector>


#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>

struct sensor_data {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
};


namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();
    enum curve_e {
        AX = 0,
        AY,
        AZ,
        GX,
        GY,
        GZ
    };

    void addpoint(double _ax, double _ay, double _az);

private slots:
    void connectedSlot();
    void disconnectedSlot();
    void readyReadSlot();
    void errorSlot(QAbstractSocket::SocketError);


    void on_pushButton_clicked();

protected:
    void update_curve();

private:
    Ui::Widget *ui;
    QTcpSocket *socket;
    bool isConnected;
    struct sensor_data sd;

    QwtPlot *plot;
    QwtPlotCurve *curve_ax, *curve_ay, *curve_az;
    QVector<double> *ax, *ay, *az, *at;

    void init_plot();
};

#endif // WIDGET_H
