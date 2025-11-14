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

#ifndef MQTTSERVER_H
#define MQTTSERVER_H

#include <QObject>
#include <QTimer>
#include <QHostAddress>
#include <QLoggingCategory>
#include <QSslConfiguration>

#include "mqttpacket.h"

class MqttServerPrivate;
class Subscription;

class MqttAuthorizer {
public:
    virtual ~MqttAuthorizer() = default;
    virtual Mqtt::ConnectReturnCode authorizeConnect(int serverAddressId, const QString &clientId, const QString &username, const QString &password, const QHostAddress &peerAddress) = 0;
    virtual bool authorizeSubscribe(int serverAddressId, const QString &clientId, const QString &topicFilter) = 0;
    virtual bool authorizePublish(int serverAddressId, const QString &clientId, const QString &topic) = 0;
};

class MqttServer : public QObject
{
    Q_OBJECT
public:
    explicit MqttServer(QObject *parent = nullptr);

    Mqtt::QoS maximumSubscriptionsQoS() const;
    void setMaximumSubscriptionsQoS(Mqtt::QoS maximumSubscriptionQoS);

    void setAuthorizer(MqttAuthorizer *authorizer);

    int listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 1883, const QSslConfiguration &sslConfiguration = QSslConfiguration());
    int listenWebSocket(const QHostAddress &address = QHostAddress::Any, quint16 port = 80, const QSslConfiguration &sslConfiguration = QSslConfiguration());
    QList<int> listeningAddressIds() const;
    QPair<QHostAddress, quint16> listeningAddress(int addressId);
    void close(int addressId);
    bool isListening(const QHostAddress &address, quint16 port) const;

    QStringList clients() const;
    void disconnectClient(const QString &clientId);

    // allows publishing from the server, including topics starting with $
    QHash<QString, quint16> publish(const QString &topic, const QByteArray &payload = QByteArray());

signals:
    // emitted whenever a client connects, after the mqtt connect handshake has been done.
    void clientConnected(int serverAddressId, const QString &clientId, const QString &username, const QHostAddress &clientAddress);
    // emitted whenever a client disconnects, that is, when a DISCONNECT message has been received or the keep alive timeout has been reached.
    void clientDisconnected(const QString &clientId);
    // emitted whenever a client has been seen, that is, a control message or a keep alive message has been received.
    void clientAlive(const QString &clientId);
    // emitted whenever a client subscribes, a client can also subscribe to topics starting with $ but those won't be relayed from other clients. Only internal server publishes to $ topics will be sent to subscribed clients.
    void clientSubscribed(const QString &clientId, const QString &topicFilter, Mqtt::QoS requestedQoS);
    // emitted whenever a client unsubscribes from a topic
    void clientUnsubscribed(const QString &clientId, const QString &topicFiltr);
    // emitted whenever a publish message is received from a client before the message is relayed to other clients. Topics starting with $ will be received here, but not relayed to other clients.
    void publishReceived(const QString &clientId, quint16 packetId, const QString &topic, const QByteArray &payload);
    // emitted whenever a publish message is sent to a client. Note: this might be fired often if many clients are connected and subsribed to matching topic filters.
    void published(const QString &clientId, quint16 packetId, const QString &topic, const QByteArray &payload);

private:
    MqttServerPrivate *d_ptr;
};

#endif // MQTTSERVER_H
