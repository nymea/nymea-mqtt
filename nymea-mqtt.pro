TEMPLATE = subdirs
SUBDIRS += libnymea-mqtt server client
server.depends = libnymea-mqtt
client.depends = libnymea-mqtt

!disabletests {
    SUBDIRS += tests
    tests.depends = libnymea-mqtt
} else {
    message("Build without tests")
}
