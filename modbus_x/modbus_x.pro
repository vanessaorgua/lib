# -------------------------------------------------
# Project created by QtCreator 2010-02-25T14:01:57
# -------------------------------------------------
QT += network
TARGET = modbus_x
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    RxModbus.cpp \
    form.cpp
HEADERS += RxModbus.h \
    form.h
MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build
RCC_DIR = build
OTHER_FILES += test_map.txt \
    list.txt
RESOURCES += test.qrc
FORMS += form.ui
