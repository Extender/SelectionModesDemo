#-------------------------------------------------
#
# Project created by QtCreator 2017-01-04T08:58:48
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = SelectionModesDemo
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    graphicssceneex.cpp \
    graphicsviewex.cpp \
    mainwindowex.cpp

HEADERS  += mainwindow.h \
    graphicssceneex.h \
    graphicsviewex.h \
    extcolordefs.h \
    mainwindowex.h

FORMS    += mainwindow.ui

RESOURCES += \
    resources.qrc
