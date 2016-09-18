#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork/QTcpSocket>
#include <QVector>


#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <QTime>

struct sensor_data {
    union {
        int16_t a[3];
        struct {
            int16_t ax;
            int16_t ay;
            int16_t az;
        };
    };

    union {
        int16_t g[3];
        struct {int16_t gx, gy, gz;};
    };

};

struct gyro_data {
    union {
        double g[3];
        struct {
            double gx, gy, gz;
        };
    };
};

struct acc_data {
    union {
        double a[3];
        struct {
            double ax, ay, az;
        };
    };
};

struct xr {
    double xr_ax, xr_ay, xr_az;
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

    enum {
        X = 0,
        Y,
        Z,
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
    bool gyro_calibrated;
    bool acc_est_initialized;
    struct gyro_data gyro_cali;
    struct sensor_data sd_raw;
    struct sensor_data sd;
    double a_u; // acce uniform value
    struct xr xra;
    union {
        double Rest_pre[3];
    };
    union {
        double Amn_pre[3];
        double Ayz_pre;
        double Axz_pre;
        double Axy_pre;
    };

    union {
        double Amn_now[3];
        double Ayz_now;
        double Axz_now;
        double Axy_now;
    };

    union {
        double Rgyro[3];
        double Rxgyro;
        double Rygyro;
        double Rzgryo;
    };


    QwtPlot *plot;
    QwtPlotCurve *curve_ax, *curve_ay, *curve_az;
    QVector<double> *ax, *ay, *az, *at;
    QTime g_timer;

    void init_plot();
};

#endif // WIDGET_H
