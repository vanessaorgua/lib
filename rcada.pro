# -------------------------------------------------
# Project created by QtCreator 2010-04-09T20:54:08
# -------------------------------------------------
QT += network \
    sql

# QT -= gui
TEMPLATE = lib
CONFIG += staticlib
SOURCES += modbus/RxModbus.cpp \
    melsec\melsec.cpp \
    logging/logging.cpp \
    logsqlite/logsqlite.cpp \
    ionetserver/IoNetServer.cpp \
    iodev.cpp \
    scaledev.cpp \
    alert/alert.cpp \
    melsec/melsec.cpp
HEADERS += modbus/RxModbus.h \
    melsec\melsec.cpp \
    logging/logging.h \
    logsqlite/logsqlite.h \
    iodev.h \
    ionetserver/IoNetServer.h \
    header.h \
    scaledev.h \
    alert/alert.h \
    melsec/melsec.h
MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build
RCC_DIR = build
VERSION = 0.1.0
FORMS += 

QMAKE_CXXFLAGS_RELEASE += -ffast-math

