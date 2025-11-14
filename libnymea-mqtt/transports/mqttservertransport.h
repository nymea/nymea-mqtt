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

#ifndef MQTTSERVERTRANSPORT_H
#define MQTTSERVERTRANSPORT_H

#include <QObject>
#include <QSslConfiguration>

class QTcpServer;

class MqttServerClient: public QObject
{
    Q_OBJECT
public:
    explicit MqttServerClient(QObject *parent = nullptr);
    virtual ~MqttServerClient() = default;

    virtual bool write(const QByteArray &data) = 0;
    virtual void abort() = 0;
    virtual bool isOpen() const = 0;
    virtual void flush() = 0;
    virtual void close() = 0;
    virtual QHostAddress peerAddress() const = 0;

signals:
    void dataAvailable(const QByteArray &data);
    void disconnected();
};

class MqttServerTransport : public QObject
{
    Q_OBJECT
public:
    explicit MqttServerTransport(QObject *parent = nullptr);
    virtual ~MqttServerTransport() = default;

    virtual bool listen(const QHostAddress &address, int port) = 0;
    virtual bool isListening() const = 0;
    virtual QHostAddress serverAddress() const = 0;
    virtual int serverPort() const = 0;
    virtual void close() = 0;

signals:
    void clientConnected(MqttServerClient *client);
};


#endif // MQTTSERVERTRANSPORT_H
