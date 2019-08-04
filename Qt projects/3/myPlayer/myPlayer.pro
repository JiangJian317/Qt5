#-------------------------------------------------
#
# Project created by QtCreator 2019-03-30T10:24:00
#
#-------------------------------------------------

QT       += core gui
QT += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myPlayer
TEMPLATE = app


SOURCES += main.cpp\
        mywidget.cpp \
    myplaylist.cpp \
    mylrc.cpp

HEADERS  += mywidget.h \
    myplaylist.h \
    mylrc.h

FORMS    += mywidget.ui

RESOURCES += \
    myImages.qrc
