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
