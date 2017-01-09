#-------------------------------------------------
#
# Project created by QtCreator 2016-12-20T12:02:43
#
#-------------------------------------------------

QT       += core gui
QT       += serialport
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

include( $${PWD}/../qwt/qwt.prf )

TARGET = loraapp
TEMPLATE = app


SOURCES += main.cpp\
        lorarc.cpp \
        hwcomms.cpp \
        spectrumplot.cpp \
        persistence1d.hpp

HEADERS  += lorarc.h \
            hwcomms.h \
            ../../../src/checksum.h \
            ../../../src/configMessages.h \
            spectrumplot.h

FORMS    += lorarc.ui

INCLUDEPATH += ../../../src/
DEFINES += DESKTOP_BUILD

RESOURCES += \
    icons.qrc
