QT += network \
    sql
TARGET = test2
CONFIG -= app_bundle
TEMPLATE = app
MOC_DIR = build
OBJECTS_DIR = build
UI_DIR = build
RCC_DIR = build
OTHER_FILES += ../librcada_client.a
QMAKE_LIBDIR += ../
LIBS += -lrcada_client
INCLUDEPATH += ../
HEADERS += mainw.h
FORMS += mainw.ui
SOURCES += main.cpp \
    mainw.cpp
RESOURCES += ../../pict/lib/valves.qrc \
    ../../Linovitsa/filters/mnemo/mnemo.qrc
