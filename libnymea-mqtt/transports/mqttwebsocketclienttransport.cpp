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

#include "mqttwebsocketclienttransport.h"

MqttWebSocketClientTransport::MqttWebSocketClientTransport(const QNetworkRequest &request, QObject *parent):
    MqttClientTransport(parent),
    m_request(request)
{
    m_socket = new QWebSocket(QString(), QWebSocketProtocol::VersionLatest, this);

    connect(m_socket, &QWebSocket::connected, this, &MqttClientTransport::connected);
    connect(m_socket, &QWebSocket::disconnected, this, &MqttClientTransport::disconnected);
    connect(m_socket, &QWebSocket::stateChanged, this, &MqttClientTransport::stateChanged);
    connect(m_socket, &QWebSocket::sslErrors, this, &MqttClientTransport::sslErrors);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(m_socket, &QWebSocket::errorOccurred, this, &MqttClientTransport::errorSignal);
#else
    typedef void (QWebSocket:: *errorSignal)(QAbstractSocket::SocketError);
    connect(m_socket, static_cast<errorSignal>(&QWebSocket::error), this, &MqttClientTransport::errorSignal);
#endif

    connect(m_socket, &QWebSocket::textMessageReceived, this, &MqttWebSocketClientTransport::onTextMessageReceived);
    connect(m_socket, &QWebSocket::binaryMessageReceived, this, &MqttWebSocketClientTransport::onBinaryMessageReceived);
}

void MqttWebSocketClientTransport::connectToHost()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 6, 0)
    if (!m_request.rawHeaderList().isEmpty()) {
        qCWarning(dbgClient) << "Qt versions older than 5.6 do not support HTTP request headers with web sockets. The connection may fail.";
    }
    m_socket->open(m_request.url());
#else
    m_socket->open(m_request);
#endif
}

void MqttWebSocketClientTransport::abort()
{
    m_socket->abort();
}

bool MqttWebSocketClientTransport::isOpen() const
{
    return m_socket->isValid();
}

bool MqttWebSocketClientTransport::write(const QByteArray &data)
{
    int ret = m_socket->sendBinaryMessage(data);
    return ret == data.length();
}

void MqttWebSocketClientTransport::flush()
{
    m_socket->flush();
}

void MqttWebSocketClientTransport::disconnectFromHost()
{
    m_socket->close();
}

QAbstractSocket::SocketState MqttWebSocketClientTransport::state() const
{
    return m_socket->state();
}

void MqttWebSocketClientTransport::ignoreSslErrors()
{
    m_socket->ignoreSslErrors();
}

void MqttWebSocketClientTransport::onTextMessageReceived(const QString &message)
{
    qCDebug(dbgClient()) << "Text message received:" << message;
    qCWarning(dbgClient()) << "Received a text message from the server. Doesn't look like MQTT. Closing connection.";
    m_socket->close(QWebSocketProtocol::CloseCodeProtocolError);
}

void MqttWebSocketClientTransport::onBinaryMessageReceived(const QByteArray &message)
{
    emit dataReceived(message);
}
