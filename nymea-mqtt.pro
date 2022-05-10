TEMPLATE = subdirs
SUBDIRS += libnymea-mqtt server client tests

server.depends = libnymea-mqtt
client.depends = libnymea-mqtt
tests.depends = libnymea-mqtt

