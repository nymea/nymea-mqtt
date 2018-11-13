TARGET = nymea-mqtt

QT -= gui
QT += network

CONFIG += c++11 console static shared
CONFIG -= app_bundle

target.path = /usr/lib/$$system('dpkg-architecture -q DEB_HOST_MULTIARCH')
INSTALLS += target

SOURCES += \
    mqttserver.cpp \
    mqttpacket.cpp \
    mqttsubscription.cpp \
    $$PWD/mqttclient.cpp

PRIVATE_HEADERS = \
    mqttpacket_p.h \
    mqttclient_p.h \
    mqttserver_p.h

PUBLIC_HEADERS = \
    mqttserver.h \
    mqttpacket.h \
    mqtt.h \
    mqttsubscription.h \
    mqttclient.h \

HEADERS += $$PRIVATE_HEADERS + $$PUBLIC_HEADERS
