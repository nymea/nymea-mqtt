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
