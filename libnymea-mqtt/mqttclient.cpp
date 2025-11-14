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

/*!
       \class MqttClient
       \brief A MQTT client
       \inmodule nymea-mqtt
       \ingroup mqtt

       MqttClient is used to connect to MQTT servers/brokers.
       The currently supported MQTT protocol version is 3.1.1.
       The currently supported transports are TCP socket and WebSocket with SSL encryption.
*/
#include "mqttclient.h"
#include "mqttclient_p.h"
#include "mqttpacket.h"

#include "transports/mqtttcpclienttransport.h"
#include "transports/mqttwebsocketclienttransport.h"

Q_LOGGING_CATEGORY(dbgClient, "nymea.mqtt.client")

MqttClientPrivate::MqttClientPrivate(MqttClient *q):
    QObject(q),
    q_ptr(q)
{
    qRegisterMetaType<Mqtt::ConnectReturnCode>();
    qRegisterMetaType<Mqtt::SubscribeReturnCodes>();
    qRegisterMetaType<Mqtt::ConnackFlags>();
    reconnectTimer.setSingleShot(true);
    connect(&keepAliveTimer, &QTimer::timeout, this, &MqttClientPrivate::sendPingreq);
    connect(&reconnectTimer, &QTimer::timeout, this, &MqttClientPrivate::reconnectTimerTimeout);
}

void MqttClientPrivate::connectToHost(const QString &hostName, quint16 port, bool cleanSession, bool useSsl, const QSslConfiguration &sslConfiguration)
{
    MqttTcpClientTransport *tcpTransport = new MqttTcpClientTransport(hostName, port, useSsl, sslConfiguration, this);
    connectToHost(tcpTransport, cleanSession);
}

void MqttClientPrivate::connectToHost(const QNetworkRequest &request, bool cleanSession)
{
    MqttWebSocketClientTransport *webSocketTransport = new MqttWebSocketClientTransport(request, this);
    connectToHost(webSocketTransport, cleanSession);
}

void MqttClientPrivate::connectToHost(MqttClientTransport *transport, bool cleanSession)
{
    if (this->transport != transport) {
        reconnectAttempt = 1;
        reconnectTimer.stop();

        if (this->transport) {
            this->transport->abort();
            this->transport->deleteLater();
        }

        this->transport = transport;

        connect(transport, &MqttClientTransport::connected, this, &MqttClientPrivate::onConnected);
        connect(transport, &MqttClientTransport::disconnected, this, &MqttClientPrivate::onDisconnected);
        connect(transport, &MqttClientTransport::dataReceived, this, &MqttClientPrivate::onDataReceived);
        connect(transport, &MqttClientTransport::stateChanged, this, &MqttClientPrivate::onSocketStateChanged);
        connect(transport, &MqttClientTransport::errorSignal, this, &MqttClientPrivate::onSocketError);
        connect(transport, &MqttClientTransport::sslErrors, this, &MqttClientPrivate::onSslErrors);
    }

    this->cleanSession = cleanSession;

    sessionActive = true;

    transport->connectToHost();
}

void MqttClientPrivate::disconnectFromHost()
{
    sessionActive = false;
    if (!transport || !transport->isOpen()) {
        return;
    }
    MqttPacket packet(MqttPacket::TypeDisconnect);
    transport->write(packet.serialize());
    transport->flush();
    transport->disconnectFromHost();
}

/*!
 * \brief Constructs a new MQTT client object.
 * \param clientId The client ID.
 * \param parent A QObject parent for this MqttClient.
 *
 * The clientId is usually obtained with the credentials for a server.
 */
MqttClient::MqttClient(const QString &clientId, QObject *parent):
    QObject(parent),
    d_ptr(new MqttClientPrivate(this))
{
    d_ptr->clientId = clientId;

}

/*!
 * \brief Constructs a new MQTT client object.
 * \param clientId The client ID.
 * \param keepAlive The keep alive timeout in seconds
 * \param willTopic The will topic for this connection
 * \param willMessage The will message payload for this connection
 * \param willQoS The QoS used to send the will for this message
 * \param willRetain Determines whether the will message should be retained on the server
 * \param parent A QObject parent for this MqttClient.
 *
 * The clientId is usually obtained with the credentials for a server. Please refer to the MQTT documentation
 * for information about how the will message in MQTT works.
 */
MqttClient::MqttClient(const QString &clientId, quint16 keepAlive, const QString &willTopic, const QByteArray &willMessage, Mqtt::QoS willQoS, bool willRetain, QObject *parent):
    QObject(parent),
    d_ptr(new MqttClientPrivate(this))
{

    d_ptr->clientId = clientId;
    d_ptr->keepAlive = keepAlive;
    d_ptr->willTopic = willTopic;
    d_ptr->willMessage = willMessage;
    d_ptr->willQoS = willQoS;
    d_ptr->willRetain = willRetain;
}

bool MqttClient::autoReconnect() const
{
    return d_ptr->autoReconnect;
}

void MqttClient::setAutoReconnect(bool autoReconnect)
{
    d_ptr->autoReconnect = autoReconnect;
}

quint16 MqttClient::maxAutoReconnectTimeout() const
{
    return d_ptr->maxReconnectTimeout;
}

void MqttClient::setMaxAutoReconnectTimeout(quint16 maxAutoReconnectTimeout)
{
    d_ptr->maxReconnectTimeout = maxAutoReconnectTimeout;
}

void MqttClient::setKeepAlive(quint16 keepAlive)
{
    d_ptr->keepAlive = keepAlive;
}

QString MqttClient::willTopic() const
{
    return d_ptr->willTopic;
}

void MqttClient::setWillTopic(const QString &willTopic)
{
    d_ptr->willTopic = willTopic;
}

QByteArray MqttClient::willMessage() const
{
    return d_ptr->willMessage;
}

void MqttClient::setWillMessage(const QByteArray &willMessage)
{
    d_ptr->willMessage = willMessage;
}

Mqtt::QoS MqttClient::willQoS() const
{
    return d_ptr->willQoS;
}

void MqttClient::setWillQoS(Mqtt::QoS willQoS)
{
    d_ptr->willQoS = willQoS;
}

bool MqttClient::willRetain() const
{
    return d_ptr->willRetain;
}

void MqttClient::setWillRetain(bool willRetain)
{
    d_ptr->willRetain = willRetain;
}

QString MqttClient::username() const
{
    return d_ptr->username;
}

void MqttClient::setUsername(const QString &username)
{
    d_ptr->username = username;
}

QString MqttClient::password() const
{
    return d_ptr->password;
}

void MqttClient::setPassword(const QString &password)
{
    d_ptr->password = password;
}

void MqttClient::connectToHost(const QString &hostName, quint16 port, bool cleanSession, bool useSsl, const QSslConfiguration &sslConfiguration)
{
    d_ptr->connectToHost(hostName, port, cleanSession, useSsl, sslConfiguration);
}

void MqttClient::connectToHost(const QNetworkRequest &request, bool cleanSession)
{
    d_ptr->connectToHost(request, cleanSession);
}

void MqttClient::disconnectFromHost()
{
    d_ptr->disconnectFromHost();
}

bool MqttClient::isConnected() const
{
    return d_ptr->transport && d_ptr->transport->state() == QAbstractSocket::ConnectedState && d_ptr->keepAliveTimer.isActive();
}

void MqttClient::ignoreSslErrors()
{
    d_ptr->transport->ignoreSslErrors();
}

quint16 MqttClient::subscribe(const MqttSubscription &subscription)
{
    MqttSubscriptions subscriptions = {subscription};
    return subscribe(subscriptions);
}

quint16 MqttClient::subscribe(const QString &topicFilter, Mqtt::QoS qos)
{
    MqttSubscription subscription(topicFilter.toUtf8(), qos);
    return subscribe(subscription);
}

quint16 MqttClient::subscribe(const MqttSubscriptions &subscriptions)
{
    MqttPacket packet(MqttPacket::TypeSubscribe, d_ptr->newPacketId());
    packet.setSubscriptions(subscriptions);
    d_ptr->unackedPackets.insert(packet.packetId(), packet);
    d_ptr->unackedPacketList.append(packet.packetId());
    d_ptr->transport->write(packet.serialize());
    return packet.packetId();
}

quint16 MqttClient::unsubscribe(const MqttSubscription &subscription)
{
    MqttSubscriptions subscriptions = {subscription};
    return unsubscribe(subscriptions);
}

quint16 MqttClient::unsubscribe(const QString &topicFilter)
{
    return unsubscribe(MqttSubscription(topicFilter.toUtf8(), Mqtt::QoS0));
}

quint16 MqttClient::unsubscribe(const MqttSubscriptions &subscriptions)
{
    MqttPacket packet(MqttPacket::TypeUnsubscribe, d_ptr->newPacketId());
    packet.setSubscriptions(subscriptions);
    d_ptr->unackedPackets.insert(packet.packetId(), packet);
    d_ptr->unackedPacketList.append(packet.packetId());
    d_ptr->transport->write(packet.serialize());
    return packet.packetId();
}

quint16 MqttClient::publish(const QString &topic, const QByteArray &payload, Mqtt::QoS qos, bool retain)
{
    quint16 packetId = qos >= Mqtt::QoS1 ? d_ptr->newPacketId() : 0;
    MqttPacket packet(MqttPacket::TypePublish, packetId, qos, retain, false);
    packet.setTopic(topic.toUtf8());
    packet.setPayload(payload);
    d_ptr->transport->write(packet.serialize());
    if (qos == Mqtt::QoS0) {
        QTimer::singleShot(0, this, [this, packet](){
            emit published(packet.packetId(), packet.topic());
        });
    } else {
        d_ptr->unackedPackets.insert(packet.packetId(), packet);
        d_ptr->unackedPacketList.append(packetId);
    }
    return packetId;
}

void MqttClientPrivate::onConnected()
{
    MqttPacket packet(MqttPacket::TypeConnect);
    packet.setProtocolLevel(Mqtt::Protocol311);
    packet.setCleanSession(cleanSession);
    packet.setKeepAlive(keepAlive);
    packet.setClientId(clientId.toUtf8());
    packet.setWillTopic(willTopic.toUtf8());
    packet.setWillMessage(willMessage);
    packet.setWillQoS(willQoS);
    packet.setWillRetain(willRetain);
    packet.setUsername(username.toUtf8());
    packet.setPassword(password.toUtf8());
    transport->write(packet.serialize());
}

void MqttClientPrivate::onDisconnected()
{
    qCDebug(dbgClient) << "Disconnected from server";
    emit q_ptr->disconnected();
    if (sessionActive && autoReconnect) {
        reconnectAttempt = qMin(maxReconnectTimeout / 60 / 60, reconnectAttempt * 2);
        qCDebug(dbgClient) << "Reconnecting in" << reconnectAttempt << "seconds";
        reconnectTimer.setInterval(reconnectAttempt * 1000);
        reconnectTimer.start();
    }
}

void MqttClientPrivate::onDataReceived(const QByteArray &data)
{
    inputBuffer.append(data);
//    qCDebug(dbgClient) << "Received data from server:" << data.toHex() << "\n" << data;
    MqttPacket packet;
    int ret = packet.parse(inputBuffer);
    if (ret == -1) {
        qCDebug(dbgClient) << "Bad data from server. Dropping connection.";
        inputBuffer.clear();
        transport->abort();
        return;
    }
    if (ret == 0) {
        qCDebug(dbgClient) << "Not enough data from server...";
        return;
    }
    inputBuffer.remove(0, ret);

    switch (packet.type()) {
    case MqttPacket::TypeConnack:
        if (packet.connectReturnCode() != Mqtt::ConnectReturnCodeAccepted) {
            qCWarning(dbgClient) << "MQTT connection refused:" << packet.connectReturnCode();
            // Always emit connected, even if just to indicate a "ClientRefusedError"
            emit q_ptr->connected(packet.connectReturnCode(), packet.connackFlags());
            transport->abort();
            emit q_ptr->error(QAbstractSocket::ConnectionRefusedError);
            return;
        }
        foreach (quint16 retryPacketId, unackedPacketList) {
            MqttPacket retryPacket = unackedPackets.value(retryPacketId);
            if (retryPacket.type() == MqttPacket::TypePublish) {
                retryPacket.setDup(true);
            }
            transport->write(retryPacket.serialize());
        }
        restartKeepAliveTimer();
        // Make sure we emit connected after having handled all the retransmission queue
        emit q_ptr->connected(packet.connectReturnCode(), packet.connackFlags());
        break;
    case MqttPacket::TypePublish:
        qCDebug(dbgClient) << "Publish received from server. Topic:" << packet.topic() << "Payload:" << packet.payload() << "QoS:" << packet.qos();
        switch (packet.qos()) {
        case Mqtt::QoS0:
            emit q_ptr->publishReceived(packet.topic(), packet.payload(), packet.retain());
            break;
        case Mqtt::QoS1: {
            emit q_ptr->publishReceived(packet.topic(), packet.payload(), packet.retain());
            MqttPacket response(MqttPacket::TypePuback, packet.packetId());
            transport->write(response.serialize());
            break;
        }
        case Mqtt::QoS2: {
            if (!packet.dup() && unackedPacketList.contains(packet.packetId())) {
                // Hmm... Server says it's not a duplicate, but packet id is not released yet... Drop connection.
                transport->disconnectFromHost();
                return;
            }

            MqttPacket response(MqttPacket::TypePubrec, packet.packetId());

            if (!unackedPacketList.contains(packet.packetId())) {
                unackedPackets.insert(packet.packetId(), response);
                unackedPacketList.append(packet.packetId());
                emit q_ptr->publishReceived(packet.topic(), packet.payload(), packet.retain());
            }
            transport->write(response.serialize());
            break;
        }
        }
        break;
    case MqttPacket::TypePuback: {
        MqttPacket publishPacket = unackedPackets.take(packet.packetId());
        unackedPacketList.removeAll(packet.packetId());
        emit q_ptr->published(packet.packetId(), publishPacket.topic());
        restartKeepAliveTimer();
        break;
    }
    case MqttPacket::TypePubrec: {
        MqttPacket publishPacket = unackedPackets.value(packet.packetId());
        MqttPacket response(MqttPacket::TypePubrel, packet.packetId());
        unackedPackets[packet.packetId()] = response;
        transport->write(response.serialize());
        emit q_ptr->published(packet.packetId(), publishPacket.topic());
        restartKeepAliveTimer();
        break;
    }
    case MqttPacket::TypePubrel: {
        MqttPacket response(MqttPacket::TypePubcomp, packet.packetId());
        unackedPackets[packet.packetId()] = response;
        transport->write(response.serialize());
        restartKeepAliveTimer();
        break;
    }
    case MqttPacket::TypePubcomp:
        unackedPackets.remove(packet.packetId());
        unackedPacketList.removeAll(packet.packetId());
        restartKeepAliveTimer();
        break;
    case MqttPacket::TypeSuback: {
        MqttPacket subscribePacket = unackedPackets.take(packet.packetId());
        unackedPacketList.removeAll(packet.packetId());

        if (subscribePacket.subscriptions().count() != packet.subscribeReturnCodes().count()) {
            qCWarning(dbgClient) << "Subscription return code count not matching subscribe packet!";
            transport->abort();
            return;
        }

        // Ack the subscription packet
        emit q_ptr->subscribeResult(packet.packetId(), packet.subscribeReturnCodes());

        // emit subscribed for each topic
        for (int i = 0; i < packet.subscribeReturnCodes().count(); i++) {
            emit q_ptr->subscribed(subscribePacket.subscriptions().at(i).topicFilter(), packet.subscribeReturnCodes().at(i));
        }
        restartKeepAliveTimer();
        break;
    }
    case MqttPacket::TypeUnsuback:
        if (!unackedPackets.contains(packet.packetId())) {
            qCWarning(dbgClient) << "UNSUBACK received but not waiting for it. Dropping connection. Packet ID:" << packet.packetId();
            transport->abort();
            return;
        }
        unackedPackets.remove(packet.packetId());
        unackedPacketList.removeAll(packet.packetId());
        emit q_ptr->unsubscribed(packet.packetId());
        restartKeepAliveTimer();
        break;
    case MqttPacket::TypePingresp:
        break;
    default:
        qCDebug(dbgClient).noquote().nospace() << "Unhandled packet type: 0x" << QString::number(packet.type(), 16);
        Q_ASSERT(false);
    }

    if (!inputBuffer.isEmpty()) {
        onDataReceived(QByteArray());
    }
}

void MqttClientPrivate::onSocketStateChanged(QAbstractSocket::SocketState socketState)
{
    emit q_ptr->stateChanged(socketState);
}

void MqttClientPrivate::onSocketError(QAbstractSocket::SocketError error)
{
    qCWarning(dbgClient) << "MQTT socket error:" << error;
    emit q_ptr->error(error);
}

void MqttClientPrivate::onSslErrors(const QList<QSslError> &errors)
{
    qCWarning(dbgClient) << "SSL error in MQTT connection:" << errors;
    emit q_ptr->sslErrors(errors);
}

quint16 MqttClientPrivate::newPacketId()
{
    static quint16 packetId = 1;
    do {
        packetId++;
    } while (unackedPacketList.contains(packetId));
    return packetId;
}

void MqttClientPrivate::sendPingreq()
{
    MqttPacket packet(MqttPacket::TypePingreq);
    transport->write(packet.serialize());
}

void MqttClientPrivate::restartKeepAliveTimer()
{
    if (keepAlive > 0) {
        keepAliveTimer.start(keepAlive * 1000);
    }
}

void MqttClientPrivate::reconnectTimerTimeout()
{
    qCDebug(dbgClient()) << "Reconnecting now...";
    if (!autoReconnect) {
        return;
    }
    connectToHost(transport, false);
}
