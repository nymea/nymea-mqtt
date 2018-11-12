/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
