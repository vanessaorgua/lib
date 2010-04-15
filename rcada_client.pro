# -------------------------------------------------
# Project created by QtCreator 2010-04-13T22:02:46
# -------------------------------------------------
QT += network \
    sql
TARGET = rcada_client
TEMPLATE = lib
CONFIG += staticlib
HEADERS += datetimedialog.h \
    trend.h \
    ionetclient/IoNetClient.h \
    iodev.h \
    header.h
SOURCES += trend.cpp \
    ionetclient/IoNetClient.cpp
FORMS += trend.ui \
    datetime.ui
MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build
RCC_DIR = build
VERSION = 0.1.0
