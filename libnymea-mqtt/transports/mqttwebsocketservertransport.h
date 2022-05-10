/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2022, nymea GmbH
* Contact: contact@nymea.io
*
* This file is part of nymea.
* This project including source code and documentation is protected by copyright law, and
* remains the property of nymea GmbH. All rights, including reproduction, publication,
* editing and translation, are reserved. The use of this project is subject to the terms of a
* license agreement to be concluded with nymea GmbH in accordance with the terms
* of use of nymea GmbH, available under https://nymea.io/license
*
* GNU Lesser General Public License Usage
* Alternatively, this project may be redistributed and/or modified under the terms of the GNU
* Lesser General Public License as published by the Free Software Foundation; version 3.
* this project is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
* without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License along with this project.
* If not, see <https://www.gnu.org/licenses/>.
*
* For any further details and any questions please contact us under contact@nymea.io
* or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
