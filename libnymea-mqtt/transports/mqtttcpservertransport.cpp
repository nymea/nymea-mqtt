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

#include "mqtttcpservertransport.h"

#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(dbgServer)


SslServer::SslServer(const QSslConfiguration &config, QObject *parent):
    QTcpServer(parent),
    m_config(config)
{

}

void SslServer::incomingConnection(qintptr socketDescriptor)
{
    QSslSocket *sslSocket = new QSslSocket(this);

    qCDebug(dbgServer) << "New client socket connection:" << sslSocket;

    connect(sslSocket, &QSslSocket::encrypted, [this, sslSocket](){ emit clientConnected(sslSocket); });
    connect(sslSocket, &QSslSocket::disconnected, this, &SslServer::onClientDisconnected);

    if (!sslSocket->setSocketDescriptor(socketDescriptor)) {
        qCWarning(dbgServer) << "Failed to set SSL socket descriptor.";
        delete sslSocket;
        return;
    }
    if (!m_config.isNull()) {
        sslSocket->setSslConfiguration(m_config);
        sslSocket->startServerEncryption();
    } else {
        emit clientConnected(sslSocket);
    }
}

void SslServer::onClientDisconnected()
{
    QSslSocket *socket = static_cast<QSslSocket*>(sender());
    qCDebug(dbgServer) << "Client socket disconnected:" << socket;
    emit clientDisconnected(socket);
    socket->deleteLater();
}

MqttTcpServerClient::MqttTcpServerClient(QTcpSocket *socket, QObject *parent):
    MqttServerClient(parent),
    m_socket(socket)
{
    m_socket->setParent(this);
    connect(socket, &QTcpSocket::readyRead, this, &MqttTcpServerClient::onSocketReadyRead);
    connect(socket, &QTcpSocket::disconnected, this, &MqttTcpServerClient::disconnected);
}

void MqttTcpServerClient::onSocketReadyRead()
{
    emit dataAvailable(m_socket->readAll());
}

bool MqttTcpServerClient::write(const QByteArray &data)
{
    qint64 len = m_socket->write(data);
    return len == data.length();
}

void MqttTcpServerClient::abort()
{
    m_socket->abort();
}

bool MqttTcpServerClient::isOpen() const
{
    return m_socket->isOpen();
}

void MqttTcpServerClient::flush()
{
    m_socket->flush();
}

void MqttTcpServerClient::close()
{
    m_socket->close();
}

QHostAddress MqttTcpServerClient::peerAddress() const
{
    return m_socket->peerAddress();
}

MqttTcpServerTransport::MqttTcpServerTransport(const QSslConfiguration &config, QObject *parent):
    MqttServerTransport(parent),
    m_sslServer(new SslServer(config, this))
{
    connect(m_sslServer, &SslServer::clientConnected, this, &MqttTcpServerTransport::onClientConnected);
}

bool MqttTcpServerTransport::listen(const QHostAddress &address, int port)
{
    return m_sslServer->listen(address, port);
}

bool MqttTcpServerTransport::isListening() const
{
    return m_sslServer->isListening();
}

QHostAddress MqttTcpServerTransport::serverAddress() const
{
    return m_sslServer->serverAddress();
}

int MqttTcpServerTransport::serverPort() const
{
    return m_sslServer->serverPort();
}

void MqttTcpServerTransport::close()
{
    return m_sslServer->close();
}

void MqttTcpServerTransport::onClientConnected(QTcpSocket *socket)
{
    MqttTcpServerClient *client = new MqttTcpServerClient(socket, this);
    emit clientConnected(client);
}
