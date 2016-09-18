#-------------------------------------------------
#
# Project created by QtCreator 2016-09-17T18:47:53
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = 6axi-client
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    mymath.c

HEADERS  += widget.h \
    mymath.h

FORMS    += widget.ui


INCLUDEPATH += /usr/local/qwt-6.1.2/lib/
INCLUDEPATH += /usr/local/qwt-6.1.2/include/
LIBS += -L/usr/local/qwt-6.1.2/lib -lqwt
