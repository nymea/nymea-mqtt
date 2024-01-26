include(../common/common.pri)

TARGET = nymeamqtttestswebsocket

SOURCES += test_websocket.cpp

target.path = $$[QT_INSTALL_PREFIX]/share/tests/nymea-mqtt/
INSTALLS += target
