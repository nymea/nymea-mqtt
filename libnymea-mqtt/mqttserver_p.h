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
class SslServer;

class MqttServerPrivate: public QObject
{
    Q_OBJECT
public:
    explicit MqttServerPrivate(MqttServer *q);

    QHash<QString, quint16> publish(const QString &topic, const QByteArray &payload = QByteArray());

public:
    void cleanupClient(QTcpSocket *client);

    void processPacket(const MqttPacket &packet, QTcpSocket *client);
    bool validateTopicFilter(const QString &topicFilter);
    bool matchTopic(const QString &topicFilter, const QString &topic);
    quint16 newPacketId(ClientContext *ctx);

public slots:
    void onClientConnected(QSslSocket *client);
    void onDataAvailable(QSslSocket *client, const QByteArray &data);
    void onClientDisconnected(QSslSocket *client);

public:
    MqttServer *q_ptr;

    QHash<int, SslServer*> servers;
    MqttAuthorizer *authorizer = nullptr;

    Mqtt::QoS maximumSubscriptionQoS = Mqtt::QoS2;

    QHash<QTcpSocket*, QTimer*> pendingConnections;
    QHash<QTcpSocket*, ClientContext*> clientList;
    QHash<QTcpSocket*, QByteArray> clientBuffers;
    QHash<QString, MqttPackets> retainedMessages;
    QHash<QTcpSocket*, SslServer*> clientServerMap;
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

class SslServer: public QTcpServer
{
    Q_OBJECT
public:
    SslServer(const QSslConfiguration &config, QObject *parent = nullptr):
        QTcpServer(parent),
        m_config(config)
    {

    }

signals:
    void clientConnected(QSslSocket *socket);
    void clientDisconnected(QSslSocket *socket);
    void dataAvailable(QSslSocket *socket, const QByteArray &data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientDisconnected();
    void onSocketReadyRead();

private:
    QSslConfiguration m_config;
};

#endif // MQTTSERVER_P_H
