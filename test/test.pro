# -------------------------------------------------
# Project created by QtCreator 2010-02-25T14:01:57
# -------------------------------------------------
QT += network \
    sql

QT -= gui

TARGET = test
CONFIG -= app_bundle
TEMPLATE = app

SOURCES += main.cpp

# MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build
RCC_DIR = build

OTHER_FILES += test_map.txt \
    list.txt \
    ../librcada.a

RESOURCES += test.qrc

QMAKE_LIBDIR += ../

LIBS += -lrcada

INCLUDEPATH += ../include

