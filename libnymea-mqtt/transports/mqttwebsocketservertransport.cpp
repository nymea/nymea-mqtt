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

#include "mqttwebsocketservertransport.h"

#include <QWebSocket>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dbgServer)

MqttWebSocketServerClient::MqttWebSocketServerClient(QWebSocket *socket, QObject *parent):
    MqttServerClient(parent),
    m_socket(socket)
{
    m_socket->setParent(this);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &MqttWebSocketServerClient::onTextMessageReceived);
    connect(m_socket, &QWebSocket::binaryMessageReceived, this, &MqttWebSocketServerClient::onBinaryMessageReceived);
    connect(m_socket, &QWebSocket::disconnected, this, &MqttServerClient::disconnected);
}

bool MqttWebSocketServerClient::write(const QByteArray &data)
{
    qint64 len = m_socket->sendBinaryMessage(data);
    return len == data.length();
}

void MqttWebSocketServerClient::abort()
{
    m_socket->abort();
}

bool MqttWebSocketServerClient::isOpen() const
{
    return m_socket->isValid();
}

void MqttWebSocketServerClient::flush()
{
    m_socket->flush();
}

void MqttWebSocketServerClient::close()
{
    m_socket->close();
}

QHostAddress MqttWebSocketServerClient::peerAddress() const
{
    return m_socket->peerAddress();
}

void MqttWebSocketServerClient::onTextMessageReceived(const QString &message)
{
    qCWarning(dbgServer).nospace() << "WebSocket received a text message from " << peerAddress() << ": " << message << ". This is not valid. Closing connection.";
    m_socket->abort();
}

void MqttWebSocketServerClient::onBinaryMessageReceived(const QByteArray &data)
{
    emit dataAvailable(data);
}

MqttWebSocketServerTransport::MqttWebSocketServerTransport(const QSslConfiguration &sslConfiguration, QObject *parent):
    MqttServerTransport(parent)
{
    if (sslConfiguration.isNull()) {
        m_server = new QWebSocketServer("nymea-mqtt", QWebSocketServer::NonSecureMode, this);
    } else {
        m_server = new QWebSocketServer("nymea-mqtt", QWebSocketServer::SecureMode, this);
        m_server->setSslConfiguration(sslConfiguration);
    }
    connect(m_server, &QWebSocketServer::newConnection, this, &MqttWebSocketServerTransport::onNewConnection);
}

bool MqttWebSocketServerTransport::listen(const QHostAddress &address, int port)
{
    return m_server->listen(address, port);
}

bool MqttWebSocketServerTransport::isListening() const
{
    return m_server->isListening();
}

QHostAddress MqttWebSocketServerTransport::serverAddress() const
{
    return m_server->serverAddress();
}

int MqttWebSocketServerTransport::serverPort() const
{
    return m_server->serverPort();
}

void MqttWebSocketServerTransport::close()
{
    m_server->close();
}

void MqttWebSocketServerTransport::onNewConnection()
{
    QWebSocket *webSocket = m_server->nextPendingConnection();
    if (!webSocket) {
        qCWarning(dbgServer()) << "New connection signalled but no pending socket available";
        return;
    }
    MqttWebSocketServerClient *client = new MqttWebSocketServerClient(webSocket, this);
    emit clientConnected(client);
}

