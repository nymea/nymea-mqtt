/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
