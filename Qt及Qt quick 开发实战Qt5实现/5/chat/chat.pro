#-------------------------------------------------
#
# Project created by QtCreator 2019-04-13T14:58:17
#
#-------------------------------------------------

QT       += core gui
QT += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = chat
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    tcpserver.cpp \
    tcpclient.cpp

HEADERS  += widget.h \
    tcpserver.h \
    tcpclient.h

FORMS    += widget.ui \
    tcpserver.ui \
    tcpclient.ui

RESOURCES += \
    images.qrc
RC_ICONS = myico.ico
