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

#ifndef MQTTTCPSERVERTRANSPORT_H
#define MQTTTCPSERVERTRANSPORT_H

#include "mqttservertransport.h"

#include <QObject>
#include <QTcpServer>

class SslServer: public QTcpServer
{
    Q_OBJECT
public:
    SslServer(const QSslConfiguration &config, QObject *parent = nullptr);

signals:
    void clientConnected(QSslSocket *socket);
    void clientDisconnected(QSslSocket *socket);
    void dataAvailable(QSslSocket *socket, const QByteArray &data);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onClientDisconnected();

private:
    QSslConfiguration m_config;
};

class MqttTcpServerClient: public MqttServerClient
{
    Q_OBJECT
public:
    explicit MqttTcpServerClient(QTcpSocket *socket, QObject *parent = nullptr);

    bool write(const QByteArray &data) override;
    void abort() override;
    bool isOpen() const override;
    void flush() override;
    void close() override;
    QHostAddress peerAddress() const override;

private slots:
    void onSocketReadyRead();

private:
    QTcpSocket *m_socket = nullptr;
};

class MqttTcpServerTransport: public MqttServerTransport
{
    Q_OBJECT
public:
    explicit MqttTcpServerTransport(const QSslConfiguration &config, QObject *parent = nullptr);

    bool listen(const QHostAddress &address, int port) override;
    bool isListening() const override;
    QHostAddress serverAddress() const override;
    int serverPort() const override;
    void close() override;

private slots:
    void onClientConnected(QTcpSocket *socket);

private:
    SslServer *m_sslServer = nullptr;
};

#endif // MQTTTCPSERVERTRANSPORT_H
