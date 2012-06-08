#-------------------------------------------------
#
# Project created by QtCreator 2012-03-31T17:12:04
#
#-------------------------------------------------

QT  += core
QT  -= gui

TARGET = HeadPoseEstimation
TEMPLATE = app


SOURCES += main.cpp\
    yawdetector.cpp \
    pitchdetector.cpp \
    landmarkreader.cpp \
    yawtrainer.cpp \
    pitchtrainer.cpp \

HEADERS  += \
    yawdetector.h \
    image.h \
    pitchdetector.h \
    landmarkreader.h \
    yawtrainer.h \
    pitchtrainer.h \

FORMS    +=

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += opencv

QMAKE_CXXFLAGS += -std=c++0x
