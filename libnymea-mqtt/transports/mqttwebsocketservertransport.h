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

#ifndef MQTTWEBSOCKETSERVERTRANSPORT_H
#define MQTTWEBSOCKETSERVERTRANSPORT_H

#include "mqttservertransport.h"

#include <QWebSocketServer>

class MqttWebSocketServerClient: public MqttServerClient
{
    Q_OBJECT
public:
    explicit MqttWebSocketServerClient(QWebSocket *socket, QObject *parent = nullptr);

    bool write(const QByteArray &data) override;
    void abort() override;
    bool isOpen() const override;
    void flush() override;
    void close() override;
    QHostAddress peerAddress() const override;

private slots:
    void onTextMessageReceived(const QString &message);
    void onBinaryMessageReceived(const QByteArray &data);

private:
    QWebSocket *m_socket = nullptr;

};

class MqttWebSocketServerTransport : public MqttServerTransport
{
    Q_OBJECT
public:
    explicit MqttWebSocketServerTransport(const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);

    bool listen(const QHostAddress &address, int port) override;
    bool isListening() const override;
    QHostAddress serverAddress() const override;
    int serverPort() const override;
    void close() override;

signals:

private slots:
    void onNewConnection();

private:
    QWebSocketServer *m_server = nullptr;

};

#endif // MQTTWEBSOCKETSERVERTRANSPORT_H
