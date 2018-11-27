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

#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QAbstractSocket>

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

    void connectToHost(const QString &hostName, quint16 port, bool cleanSession = true);
    void disconnectFromHost();

    bool isConnected() const;

public slots:
    quint16 subscribe(const MqttSubscription &subscription);
    quint16 subscribe(const QString &topciFilter, Mqtt::QoS qos = Mqtt::QoS0);
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

    void subscribed(quint16 packetId, const Mqtt::SubscribeReturnCodes &subscribeReturnCodes);
    void unsubscribed(quint16 packetId);
    void published(quint16 packetId);
    void publishReceived(const QString &topic, const QByteArray &payload, bool retained);

private:
    MqttClientPrivate *d_ptr;
    friend class OperationTests;
};

#endif // MQTTCLIENT_H
