# Include this file in your project if you want to 
# statically link to libnymea-mqtt

QT -= gui
QT += network

CONFIG += c++11 console static
CONFIG -= app_bundle

SOURCES += \
    mqttserver.cpp \
    mqttpacket.cpp \
    mqttsubscription.cpp \
    mqttclient.cpp

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
