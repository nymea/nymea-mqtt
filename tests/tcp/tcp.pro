TARGET = nymeamqttteststcp

include(../common/common.pri)

SOURCES += test_tcp.cpp

target.path = $$[QT_INSTALL_PREFIX]/share/tests/nymea-mqtt/
INSTALLS += target
