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

#include "mqtttcpclienttransport.h"

MqttTcpClientTransport::MqttTcpClientTransport(const QString &hostName, quint16 port, bool useSsl, const QSslConfiguration &sslConfiguration, QObject *parent):
    MqttClientTransport(parent),
    m_hostName(hostName),
    m_port(port),
    m_useSsl(useSsl)
{
    m_socket = new QSslSocket(this);
    m_socket->setSslConfiguration(sslConfiguration);

    connect(m_socket, &QTcpSocket::connected, this, &MqttClientTransport::connected);
    connect(m_socket, &QTcpSocket::disconnected, this, &MqttClientTransport::disconnected);
    connect(m_socket, &QTcpSocket::stateChanged, this, &MqttClientTransport::stateChanged);
    connect(m_socket, &QTcpSocket::readyRead, this, &MqttTcpClientTransport::onReadyRead);

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    connect(m_socket, &QSslSocket::sslErrors, this, &MqttClientTransport::sslErrors);
#else
    typedef void (QSslSocket:: *sslErrorsSignal)(const QList<QSslError> &);
    connect(m_socket, static_cast<sslErrorsSignal>(&QSslSocket::sslErrors), this, &MqttClientTransport::sslErrors);
#endif

#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_socket, &QTcpSocket::errorOccurred, this, &MqttClientTransport::errorSignal);
#else
    typedef void (QSslSocket:: *errorSignal)(QAbstractSocket::SocketError);
    connect(m_socket, static_cast<errorSignal>(&QSslSocket::error), this, &MqttClientTransport::errorSignal);
#endif
}

void MqttTcpClientTransport::connectToHost()
{
    if (m_useSsl) {
        m_socket->connectToHostEncrypted(m_hostName, m_port);
    } else {
        m_socket->connectToHost(m_hostName, m_port);
    }
}

void MqttTcpClientTransport::abort()
{
    m_socket->abort();
}

bool MqttTcpClientTransport::isOpen() const
{
    return m_socket->isOpen();
}

bool MqttTcpClientTransport::write(const QByteArray &data)
{
    int ret = m_socket->write(data);
    return ret == data.length();
}

void MqttTcpClientTransport::flush()
{
    m_socket->flush();
}

void MqttTcpClientTransport::disconnectFromHost()
{
    m_socket->disconnectFromHost();
}

QAbstractSocket::SocketState MqttTcpClientTransport::state() const
{
    return m_socket->state();
}

void MqttTcpClientTransport::ignoreSslErrors()
{
    m_socket->ignoreSslErrors();
}

void MqttTcpClientTransport::onReadyRead()
{
    QByteArray data = m_socket->readAll();
    emit dataReceived(data);
}
