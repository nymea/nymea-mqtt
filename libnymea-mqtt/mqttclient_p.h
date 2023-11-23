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

#ifndef MQTTCLIENT_P_H
#define MQTTCLIENT_P_H

#include <QObject>
#include <QTcpSocket>
#include <QWebSocket>
#include <QTimer>
#include <QLoggingCategory>

#include "mqttpacket.h"
#include "mqttsubscription.h"
#include "mqttclient.h"
#include "transports/mqttclienttransport.h"

Q_DECLARE_LOGGING_CATEGORY(dbgClient)

class MqttClientPrivate: public QObject
{
    Q_OBJECT
public:
    MqttClientPrivate(MqttClient *q);
    MqttClient *q_ptr;

    void connectToHost(const QString &hostName, quint16 port, bool cleanSession, bool useSsl, const QSslConfiguration &sslConfiguration);
    void connectToHost(const QNetworkRequest &request, bool cleanSession);
    void connectToHost(MqttClientTransport *transport, bool cleanSession = true);
    void disconnectFromHost();

public slots:
    void onConnected();
    void onDisconnected();
    void onDataReceived(const QByteArray &data);
    void onSocketStateChanged(QAbstractSocket::SocketState socketState);
    void onSocketError(QAbstractSocket::SocketError error);
    void onSslErrors(const QList<QSslError> &errors);

    quint16 newPacketId();
    void sendPingreq();
    void restartKeepAliveTimer();

    void reconnectTimerTimeout();

public:
    bool autoReconnect = true;
    bool sessionActive = false;
    bool cleanSession = true;
    MqttClientTransport *transport = nullptr;
    QTimer reconnectTimer;
    int reconnectAttempt = 0;
    quint16 maxReconnectTimeout = 36000;

    QString clientId;
    quint16 keepAlive = 0;
    QTimer keepAliveTimer;
    QString willTopic;
    QByteArray willMessage;
    Mqtt::QoS willQoS = Mqtt::QoS0;
    bool willRetain = false;
    QString username;
    QString password;

    QVector<quint16> unackedPacketList;
    QHash<quint16, MqttPacket> unackedPackets;

    QByteArray inputBuffer;
};

#endif // MQTTCLIENT_P_H
