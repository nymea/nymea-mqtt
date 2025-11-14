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

#ifndef MQTTWEBSOCKETCLIENTTRANSPORT_H
#define MQTTWEBSOCKETCLIENTTRANSPORT_H

#include "mqttclienttransport.h"

class MqttWebSocketClientTransport: public MqttClientTransport
{
    Q_OBJECT
public:
    explicit MqttWebSocketClientTransport(const QNetworkRequest &request, QObject *parent = nullptr);

    void connectToHost() override;

    void abort() override;
    bool isOpen() const override;
    bool write(const QByteArray &data) override;
    void flush() override;
    void disconnectFromHost() override;
    QAbstractSocket::SocketState state() const override;
    void ignoreSslErrors() override;

private slots:
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &message);

private:
    QNetworkRequest m_request;
    QWebSocket *m_socket = nullptr;
};

#endif
