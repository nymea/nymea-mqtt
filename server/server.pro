TEMPLATE = app
TARGET = nymea-mqtt-server

include(../nymea-mqtt.pri)

QT += network
QT -= gui

INCLUDEPATH += $$top_srcdir/libnymea-mqtt/

HEADERS += \
    authorizer.h \
    certificateloader.h \
    mqttpolicy.h

SOURCES += main.cpp \
    authorizer.cpp \
    certificateloader.cpp \
    mqttpolicy.cpp

LIBS += -L$$top_builddir/libnymea-mqtt/ -lnymea-mqtt -lssl -lcrypto

target.path = $$[QT_INSTALL_PREFIX]/bin
INSTALLS += target

