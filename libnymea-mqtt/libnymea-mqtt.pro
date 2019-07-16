TEMPLATE = lib
TARGET = nymea-mqtt

include(libnymea-mqtt.pri)

# The .pri is made for other projects to include it in a static manner
# Change it to build a shared lib here.
CONFIG -= static
CONFIG += shared

# install lib
target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target

# install headers
pubheaders.files += $$PUBLIC_HEADERS
pubheaders.path = $$[QT_INSTALL_PREFIX]/include/nymea-mqtt/
INSTALLS += pubheaders

# Create pkgconfig file
CONFIG += create_pc create_prl no_install_prl
QMAKE_PKGCONFIG_NAME = nymea-mqtt
QMAKE_PKGCONFIG_DESCRIPTION = nymea mqtt library
QMAKE_PKGCONFIG_PREFIX = $$[QT_INSTALL_PREFIX]
QMAKE_PKGCONFIG_INCDIR = $$[QT_INSTALL_PREFIX]/include/nymea-mqtt/
QMAKE_PKGCONFIG_LIBDIR = $$target.path
QMAKE_PKGCONFIG_VERSION = "0.0.1"
QMAKE_PKGCONFIG_FILE = nymea-mqtt
QMAKE_PKGCONFIG_DESTDIR = pkgconfig
