#-------------------------------------------------
#
# Project created by QtCreator 2015-01-19T15:49:33
#
#-------------------------------------------------

QT       += core gui printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = ControlGUI
TEMPLATE = app


SOURCES += main.cpp\
        controlgui.cpp \
    u3.c \
    qcustomplot.cpp \
    fill_checker.cpp

HEADERS  += controlgui.h \
    u3.h \
    qcustomplot.h \
    fill_checker.h

FORMS    += controlgui.ui


LIBS += -llabjackusb
LIBS += -lzmq
