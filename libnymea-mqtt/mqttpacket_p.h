#ifndef MQTTPACKET_P_H
#define MQTTPACKET_P_H

#include <QByteArray>
#include <QByteArray>
#include <QList>
#include <QLoggingCategory>

#include "mqtt.h"
#include "mqttpacket.h"
#include "mqttsubscription.h"

Q_DECLARE_LOGGING_CATEGORY(dbgProto)

class MqttPacketPrivate
{
public:
    MqttPacketPrivate(MqttPacket *q) : q_ptr(q) { }
    MqttPacket *q_ptr;

    bool verifyHeaderFlags();

    quint8 header = 0;
    QByteArray protocolName = "MQTT";
    Mqtt::Protocol protocolLevel = Mqtt::ProtocolUnknown;
    Mqtt::ConnectFlags connectFlags = Mqtt::ConnectFlagNone;
    Mqtt::ConnackFlags connackFlags = Mqtt::ConnackFlagNone;
    quint16 keepAlive = 0;
    QByteArray clientId;
    QByteArray willTopic;
    QByteArray willMessage;
    QByteArray username;
    QByteArray password;

    quint16 packetId = 0;
    QByteArray topic;
    QByteArray payload;

    Mqtt::ConnectReturnCode connectReturnCode = Mqtt::ConnectReturnCodeAccepted;

    MqttSubscriptions subscriptions;
    Mqtt::SubscribeReturnCodes subscribeReturnCodes;
};

#endif // MQTTPACKET_P_H
