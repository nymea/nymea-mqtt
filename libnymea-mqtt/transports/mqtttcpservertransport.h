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
