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

#ifndef MQTTCLIENTTRANSPORT_H
#define MQTTCLIENTTRANSPORT_H

#include <QObject>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QWebSocket>
#include <QNetworkRequest>

#include <QLoggingCategory>
Q_DECLARE_LOGGING_CATEGORY(dbgClient)

class MqttClientTransport : public QObject
{
    Q_OBJECT
public:
    explicit MqttClientTransport(QObject *parent = nullptr);
    virtual ~MqttClientTransport() = default;

    virtual void connectToHost() = 0;
    virtual void abort() = 0;
    virtual bool isOpen() const = 0;
    virtual bool write(const QByteArray &data) = 0;
    virtual void flush() = 0;
    virtual void disconnectFromHost() = 0;
    virtual QAbstractSocket::SocketState state() const = 0;
    virtual void ignoreSslErrors() = 0;

signals:
    void connected();
    void disconnected();
    void dataReceived(const QByteArray &data);
    void stateChanged(QAbstractSocket::SocketState state);
    void errorSignal(QAbstractSocket::SocketError error);
    void sslErrors(const QList<QSslError> &sslErrors);

};

#endif // MQTTCLIENTTRANSPORT_H
