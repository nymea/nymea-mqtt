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

#ifndef MQTTPACKET_H
#define MQTTPACKET_H

#include <QByteArray>
#include <QByteArray>
#include <QList>
#include <QLoggingCategory>
#include <QSharedDataPointer>

#include "mqtt.h"
#include "mqttsubscription.h"

class MqttPacketPrivate;

class MqttPacket
{
public:

    enum Type {
        TypeConnect = 0x10,
        TypeConnack = 0x20,
        TypePublish = 0x30,
        TypePuback = 0x40,
        TypePubrec = 0x50,
        TypePubrel = 0x60,
        TypePubcomp = 0x70,
        TypeSubscribe = 0x80,
        TypeSuback = 0x90,
        TypeUnsubscribe = 0xa0,
        TypeUnsuback = 0xb0,
        TypePingreq = 0xc0,
        TypePingresp = 0xd0,
        TypeDisconnect = 0xe0
    };

    MqttPacket();
    MqttPacket(const MqttPacket &other);
    MqttPacket(Type type, quint16 packetId = 0, Mqtt::QoS qos = Mqtt::QoS0, bool retain = false, bool dup = false);
    ~MqttPacket();

    Type type() const;
    bool dup() const;
    void setDup(bool dup);
    Mqtt::QoS qos() const;
    void setQoS(Mqtt::QoS qoS);

    bool retain() const;
    void setRetain(bool retain);


    // CONNECT
    void setCleanSession(bool cleanSession);
    bool cleanSession() const;
    Mqtt::ConnectFlags connectFlags() const;
    QByteArray protocolName() const;
    Mqtt::Protocol protocolLevel() const;
    void setProtocolLevel(Mqtt::Protocol protocolLevel);
    quint16 keepAlive() const;
    void setKeepAlive(quint16 keepAlive);
    QByteArray clientId() const;
    void setClientId(const QByteArray &clientId);
    QByteArray willTopic() const;
    void setWillTopic(const QByteArray &willTopic);
    QByteArray willMessage() const;
    void setWillMessage(const QByteArray &willMessage);
    Mqtt::QoS willQoS() const;
    void setWillQoS(Mqtt::QoS willQoS);
    bool willRetain() const;
    void setWillRetain(bool willRetain);
    QByteArray username() const;
    void setUsername(const QByteArray &username);
    QByteArray password() const;
    void setPassword(const QByteArray &password);

    // CONNACK
    Mqtt::ConnectReturnCode connectReturnCode() const;
    void setConnectReturnCode(Mqtt::ConnectReturnCode connectReturnCode);

    Mqtt::ConnackFlags connackFlags() const;
    void setConnackFlags(Mqtt::ConnackFlags connackFlags);

    // PUBLISH/SUBSCRIBE
    quint16 packetId() const;
    void setPacketId(quint16 packetId);
    // PUBLISH
    QByteArray topic() const;
    void setTopic(const QByteArray &topic);
    QByteArray payload() const;
    void setPayload(const QByteArray &payload);
    // SUBSCRIBE
    MqttSubscriptions subscriptions() const;
    void setSubscriptions(const MqttSubscriptions &subscriptions);
    void addSubscription(const MqttSubscription &subscription);
    // SUBACK
    Mqtt::SubscribeReturnCodes subscribeReturnCodes() const;
    void setSubscribeReturnCodes(const Mqtt::SubscribeReturnCodes subscribeReturnCodes);
    void addSubscribeReturnCode(Mqtt::SubscribeReturnCode subscribeReturnCode);

    // Takes a buffer and fills the packet accordingly.
    // Returns the length of data used from the buffer on success, bad() will return false
    // Returns -1 on bad data input, bad() will return true
    // Returns 0 if input data is ok, but not long enough, bad() will return true
    int parse(const QByteArray &buffer);
    QByteArray serialize() const;

    bool operator==(const MqttPacket &other) const;
    MqttPacket &operator=(const MqttPacket &other);

private:
    QSharedDataPointer<MqttPacketPrivate> d_ptr;
};

typedef QList<MqttPacket> MqttPackets;

#endif // MQTTPACKET_H
