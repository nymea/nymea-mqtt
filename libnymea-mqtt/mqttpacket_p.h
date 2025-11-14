// SPDX-License-Identifier: LGPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* nymea-mqtt
* MQTT library for nymea
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-mqtt.
*
* nymea-mqtt is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* as published by the Free Software Foundation, either version 3
* of the License, or (at your option) any later version.
*
* nymea-mqtt is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with nymea-mqtt. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef MQTTPACKET_P_H
#define MQTTPACKET_P_H

#include <QByteArray>
#include <QByteArray>
#include <QList>
#include <QLoggingCategory>

#include "mqtt.h"
#include "mqttpacket.h"
#include "mqttsubscription.h"

#include <QSharedData>

Q_DECLARE_LOGGING_CATEGORY(dbgProto)

class MqttPacketPrivate: public QSharedData
{
public:
    MqttPacketPrivate(){ }
    MqttPacketPrivate(const MqttPacketPrivate &other);

    bool verifyHeaderFlags();
    MqttPacket::Type type() const;
    bool dup() const;
    Mqtt::QoS qos() const;
    bool retain() const;

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
