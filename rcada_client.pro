# -------------------------------------------------
# Project created by QtCreator 2010-04-13T22:02:46
# -------------------------------------------------
QT += network \
    sql
TEMPLATE = lib
CONFIG += staticlib uitools
HEADERS += iodev.h \
    header.h \
    ionetclient/netiodev.h \
    ionetclient/IoNetClient.h \
    trendchar/trendchart.h \
    trend/trend.h \
    trend/datetimedialog.h \
    RotatedLabel/rotatedlabel.h \
    panelReg/panelReg.h \
    scale/scale.h \
    trend/trendconstruct.h \
    history/history.h
SOURCES += ionetclient/IoNetClient.cpp \
    ionetclient/netiodev.cpp \
    iodev.cpp \
    trendchar/trendchart.cpp \
    trend/trend.cpp \
    RotatedLabel/rotatedlabel.cpp \
    panelReg/panelReg.cpp \
    scale/scale.cpp \
    trend/trendconstruct.cpp \
    history/history.cpp
MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build
RCC_DIR = build
VERSION = 0.1.0
RESOURCES += trend/trend.qrc \
    picture.qrc
FORMS += trend/trend.ui \
    trend/datetime.ui \
    panelReg/panelReg.ui \
    trend/trendconstruct.ui
INCLUDEPATH += include
