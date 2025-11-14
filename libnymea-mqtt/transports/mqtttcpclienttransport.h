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

#ifndef MQTTTCPCLIENTTRANSPORT_H
#define MQTTTCPCLIENTTRANSPORT_H

#include "mqttclienttransport.h"

class MqttTcpClientTransport: public MqttClientTransport
{
    Q_OBJECT
public:
    explicit MqttTcpClientTransport(const QString &hostName, quint16 port, bool useSsl, const QSslConfiguration &sslConfiguration, QObject *parent = nullptr);

    void connectToHost() override;

    void abort() override;
    bool isOpen() const override;
    bool write(const QByteArray &data) override;
    void flush() override;
    void disconnectFromHost() override;
    QAbstractSocket::SocketState state() const override;
    void ignoreSslErrors() override;

private slots:
    void onReadyRead();

private:
    QString m_hostName;
    quint16 m_port;
    bool m_useSsl = false;
    QSslSocket *m_socket = nullptr;
};

#endif
