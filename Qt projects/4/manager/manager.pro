#-------------------------------------------------
#
# Project created by QtCreator 2019-04-03T23:53:50
#
#-------------------------------------------------

QT       += core gui
QT       += sql xml
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = manager
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    pieview.cpp \
    logindialog.cpp

HEADERS  += widget.h \
    connection.h \
    pieview.h \
    logindialog.h

FORMS    += widget.ui \
    logindialog.ui

RC_ICONS = myico.ico
