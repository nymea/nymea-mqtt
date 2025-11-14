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

#ifndef MQTTSERVER_P_H
#define MQTTSERVER_P_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QLoggingCategory>

#include "mqttpacket.h"
#include "mqttserver.h"

Q_DECLARE_LOGGING_CATEGORY(dbgServer)

class ClientContext;
class Subscription;
class MqttServerTransport;
class MqttServerClient;

class MqttServerPrivate: public QObject
{
    Q_OBJECT
public:
    explicit MqttServerPrivate(MqttServer *q);

    int listen(MqttServerTransport *transport, const QHostAddress &address, quint16 port);
    QHash<QString, quint16> publish(const QString &topic, const QByteArray &payload = QByteArray());
    void cleanupClient(MqttServerClient *client);

    void processPacket(const MqttPacket &packet, MqttServerClient *client);
    bool validateTopicFilter(const QString &topicFilter);
    bool matchTopic(const QString &topicFilter, const QString &topic);
    quint16 newPacketId(ClientContext *ctx);

public slots:
    void onClientConnected(MqttServerClient *client);
    void onDataAvailable(const QByteArray &data);
    void onClientDisconnected();

public:
    MqttServer *q_ptr;

    QHash<int, MqttServerTransport*> servers;
    MqttAuthorizer *authorizer = nullptr;

    Mqtt::QoS maximumSubscriptionQoS = Mqtt::QoS2;

    QHash<MqttServerClient*, QTimer*> pendingConnections;
    QHash<MqttServerClient*, ClientContext*> clientList;
    QHash<MqttServerClient*, QByteArray> clientBuffers;
    QHash<QString, MqttPackets> retainedMessages;
    QHash<MqttServerClient*, MqttServerTransport*> clientServerMap;
};

class ClientContext {
public:
    Mqtt::Protocol version = Mqtt::ProtocolUnknown;
    quint16 keepAlive = 0;
    QTimer keepAliveTimer;
    QString clientId;
    QString username;
    QByteArray willTopic;
    QByteArray willMessage;
    Mqtt::QoS willQoS = Mqtt::QoS0;
    bool willRetain = false;

    QByteArray inputBuffer;
    MqttSubscriptions subscriptions;

    QVector<quint16> unackedPacketList;
    QHash<quint16, MqttPacket> unackedPackets;
};

#endif // MQTTSERVER_P_H
