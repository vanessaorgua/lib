# -------------------------------------------------
# Project created by QtCreator 2010-02-25T14:01:57
# -------------------------------------------------
QT += network
QT -= gui
TARGET = modbus_x
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    RxModbus.cpp
HEADERS += RxModbus.h
MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build
RCC_DIR = build
OTHER_FILES += ../../8=>28FO/$V;LB@8/text/tag_map.txt \
    ../../Linovitsa/filters/text/tag_list.txt \
    test_map.txt
RESOURCES += test.qrc
