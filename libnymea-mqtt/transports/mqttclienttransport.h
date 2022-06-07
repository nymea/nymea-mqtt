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
