#-------------------------------------------------
#
# Project created by QtCreator 2019-03-15T10:21:56
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myMdi
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    mdichild.cpp

HEADERS  += mainwindow.h \
    mdichild.h

FORMS    += mainwindow.ui

RESOURCES += \
    myImage.qrc

RC_ICONS = myico.ico
