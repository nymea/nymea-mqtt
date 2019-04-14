TEMPLATE = lib
TARGET = nymea-mqtt

include(libnymea-mqtt.pri)

# The .pri is made for other projects to include it in a static manner
# Change it to build a shared lib here.
CONFIG -= static
CONFIG += shared

# install lib
target.path = /usr/lib
INSTALLS += target 

# install headers
pubheaders.files += $$PUBLIC_HEADERS
pubheaders.path = /usr/include/nymea-mqtt/
INSTALLS += pubheaders

