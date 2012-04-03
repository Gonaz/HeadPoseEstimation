#-------------------------------------------------
#
# Project created by QtCreator 2012-03-31T17:12:04
#
#-------------------------------------------------

QT       += core gui

TARGET = HeadPoseEstimation
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    inputgraphicsview.cpp

HEADERS  += mainwindow.h \
    inputgraphicsview.h

FORMS    += mainwindow.ui

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv

QMAKE_CXXFLAGS += -std=c++0x
