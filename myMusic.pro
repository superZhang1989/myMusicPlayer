#-------------------------------------------------
#
# Project created by QtCreator 2014-05-04T20:03:30
#
#-------------------------------------------------

QT       += core gui
QT       += phonon

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = myMusic
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
