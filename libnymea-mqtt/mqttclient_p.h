#ifndef MQTTCLIENT_P_H
#define MQTTCLIENT_P_H

#include <QObject>
#include <QTcpSocket>
#include <QTimer>
#include <QLoggingCategory>

#include "mqttpacket.h"
#include "mqttclient.h"
#include "mqttsubscription.h"

Q_DECLARE_LOGGING_CATEGORY(dbgClient)

class MqttClientPrivate: public QObject
{
    Q_OBJECT
public:
    MqttClientPrivate(MqttClient *q);
    MqttClient *q_ptr;
    ~MqttClientPrivate() { qDebug() << "destroying client private" << this; }

    void connectToHost(const QString &hostName, quint16 port, bool cleanSession);
    void disconnectFromHost();

public slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);

    quint16 newPacketId();
    void sendPingreq();
    void restartKeepAliveTimer();

public:
    QString serverHostname;
    quint16 serverPort = 0;
    bool autoReconnect = true;
    bool sessionActive = false;
    bool cleanSession = true;
    QTcpSocket *socket = nullptr;

    QString clientId;
    quint16 keepAlive;
    QTimer keepAliveTimer;
    QString willTopic;
    QByteArray willMessage;
    Mqtt::QoS willQoS = Mqtt::QoS0;
    bool willRetain = false;
    QString username;
    QString password;

    QVector<quint16> unackedPacketList;
    QHash<quint16, MqttPacket> unackedPackets;
};

#endif // MQTTCLIENT_P_H
