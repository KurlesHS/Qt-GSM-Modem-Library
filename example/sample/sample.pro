#-------------------------------------------------
#
# Project created by QtCreator 2013-12-26T15:05:50
#
#-------------------------------------------------

QT       += core serialport

QT       -= gui

TARGET = sample
CONFIG   += console c++11
CONFIG   -= app_bundle

TEMPLATE = app
include(../../gsmmodem/gsmmodem.pri)

SOURCES += main.cpp \
    worker.cpp

HEADERS += \
    worker.h
