// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-mqtt.
*
* nymea-mqtt is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-mqtt is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-mqtt. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "mqttserver.h"
#include "mqttclient.h"

#include <QTest>
#include <QSignalSpy>

#include "../common/mqtttests.h"

class WebSocketTests: public MqttTests
{
    Q_OBJECT

private:
    int startServer(MqttServer *server) override;
    void connectClientToServer(MqttClient *client, bool cleanSession) override;

    QString m_serverHost = "127.0.0.1";
    quint16 m_serverPort = 5556;

};

int WebSocketTests::startServer(MqttServer *server)
{
    return server->listenWebSocket(QHostAddress(m_serverHost), m_serverPort);
}

void WebSocketTests::connectClientToServer(MqttClient *client, bool cleanSession)
{
    QUrl url;
    url.setScheme("ws");
    url.setHost(m_serverHost);
    url.setPort(m_serverPort);
    QNetworkRequest request(url);
    qDebug() << "Connecting to WebSocket";
    client->connectToHost(request, cleanSession);
}

QTEST_MAIN(WebSocketTests)

#include "test_websocket.moc"
