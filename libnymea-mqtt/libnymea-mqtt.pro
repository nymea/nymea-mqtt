TEMPLATE = lib
TARGET = nymea-mqtt

include(libnymea-mqtt.pri)

# install headers
pubheaders.files += $$PUBLIC_HEADERS
pubheaders.path = /usr/include/nymea-mqtt/
INSTALLS += pubheaders

