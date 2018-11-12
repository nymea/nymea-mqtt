#include <QCoreApplication>

#include "mqttserver.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    MqttServer server;
    server.listen(QHostAddress::AnyIPv4, 1883);

    return a.exec();
}
