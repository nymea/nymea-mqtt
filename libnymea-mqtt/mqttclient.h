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

#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QAbstractSocket>
#include <QSslConfiguration>
#include <QNetworkRequest>

#include "mqttpacket.h"
#include "mqttsubscription.h"

class MqttClientPrivate;

class MqttClient : public QObject
{
    Q_OBJECT
public:
    explicit MqttClient(const QString &clientId, QObject *parent = nullptr);
    explicit MqttClient(const QString &clientId, quint16 keepAlive = 300, const QString &willTopic = QString(), const QByteArray &willMessage = QByteArray(), Mqtt::QoS willQoS = Mqtt::QoS0, bool willRetain = false, QObject *parent = nullptr);

    bool autoReconnect() const;
    void setAutoReconnect(bool autoReconnect);

    quint16 maxAutoReconnectTimeout() const;
    void setMaxAutoReconnectTimeout(quint16 maxAutoReconnectTimeout);

    quint16 keepAlive() const;
    void setKeepAlive(quint16 keepAlive);

    QString willTopic() const;
    void setWillTopic(const QString &willTopic);

    QByteArray willMessage() const;
    void setWillMessage(const QByteArray &willMessage);

    Mqtt::QoS willQoS() const;
    void setWillQoS(Mqtt::QoS willQoS);

    bool willRetain() const;
    void setWillRetain(bool willRetain);

    QString username() const;
    void setUsername(const QString &username);

    QString password() const;
    void setPassword(const QString &password);

    void connectToHost(const QString &hostName, quint16 port, bool cleanSession = true, bool useSsl = false, const QSslConfiguration &sslConfiguration = QSslConfiguration());
    void connectToHost(const QNetworkRequest &request, bool cleanSession = true);
    void disconnectFromHost();

    bool isConnected() const;

    void ignoreSslErrors();

public slots:
    quint16 subscribe(const MqttSubscription &subscription);
    quint16 subscribe(const QString &topicFilter, Mqtt::QoS qos = Mqtt::QoS0);
    quint16 subscribe(const MqttSubscriptions &subscriptions);

    quint16 unsubscribe(const MqttSubscription &subscription);
    quint16 unsubscribe(const QString &topicFilter);
    quint16 unsubscribe(const MqttSubscriptions &subscriptions);

    quint16 publish(const QString &topic, const QByteArray &payload, Mqtt::QoS qos = Mqtt::QoS0, bool retain = false);

signals:
    void connected(Mqtt::ConnectReturnCode connectReturnCode, Mqtt::ConnackFlags connackFlags);
    void disconnected();
    void stateChanged(QAbstractSocket::SocketState state);
    void error(QAbstractSocket::SocketError socketError);
    void sslErrors(const QList<QSslError> &sslErrors);

    void subscribeResult(quint16 packetId, const Mqtt::SubscribeReturnCodes &subscribeReturnCodes);
    void subscribed(const QString &topic, Mqtt::SubscribeReturnCode subscribeReturnCode);
    void unsubscribed(quint16 packetId);
    void published(quint16 packetId, const QString &topic);
    void publishReceived(const QString &topic, const QByteArray &payload, bool retained);

private:
    MqttClientPrivate *d_ptr;
    friend class MqttTests;
};

#endif // MQTTCLIENT_H
