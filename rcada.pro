# -------------------------------------------------
# Project created by QtCreator 2010-04-09T20:54:08
# -------------------------------------------------
QT += network \
    sql

# QT -= gui
TARGET = rcada
TEMPLATE = lib
CONFIG += staticlib
SOURCES += modbus/RxModbus.cpp \
    logging/logging.cpp \
    iodev.cpp
HEADERS += modbus/RxModbus.h \
    logging/logging.h \
    iodev.h
MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build
RCC_DIR = build
VERSION = 0.1.0
