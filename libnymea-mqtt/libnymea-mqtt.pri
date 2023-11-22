# Include this file in your project if you want to 
# statically link to libnymea-mqtt

TEMPLATE = lib
TARGET = nymea-mqtt

QT -= gui
QT += network websockets

include(../nymea-mqtt.pri)

CONFIG += console static

SOURCES += \
    mqttpacket.cpp \
    mqttsubscription.cpp \
    mqttserver.cpp \
    mqttclient.cpp \
    transports/mqttservertransport.cpp \
    transports/mqtttcpservertransport.cpp \
    transports/mqttwebsocketservertransport.cpp \
    transports/mqttclienttransport.cpp \
    transports/mqtttcpclienttransport.cpp \
    transports/mqttwebsocketclienttransport.cpp \

PRIVATE_HEADERS = \
    mqttpacket_p.h \
    mqttclient_p.h \
    mqttserver_p.h \
    transports/mqttservertransport.h \
    transports/mqtttcpservertransport.h \
    transports/mqttwebsocketservertransport.h \
    transports/mqttclienttransport.h \
    transports/mqtttcpclienttransport.h \
    transports/mqttwebsocketclienttransport.h \

PUBLIC_HEADERS = \
    mqtt.h \
    mqttpacket.h \
    mqttsubscription.h \
    mqttserver.h \
    mqttclient.h \

HEADERS += $$PRIVATE_HEADERS $$PUBLIC_HEADERS

# https://bugreports.qt.io/browse/QTBUG-83165
android: {
    DESTDIR = $${ANDROID_TARGET_ARCH}
    OBJECTS_DIR = $${ANDROID_TARGET_ARCH}
}
