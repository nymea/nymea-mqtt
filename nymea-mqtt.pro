TEMPLATE = subdirs
SUBDIRS += libnymea-mqtt server tests

server.depends = libnymea-mqtt
tests.depends = libnymea-mqtt

