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
