/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright 2013 - 2020, nymea GmbH
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

/*!
       \class MqttServer
       \brief A MQTT server implementation
       \inmodule nymea-mqtt
       \ingroup mqtt

       MqttServer is used to expose a MQTT server interface in the network. The currently supported
       MQTT protocol version is 3.1.1 including SSL encryption support.
       Note: Just starting up such a MqttServer does not provide a full MQTT broker. A MqttServer
       listens on the network for incoming connections, accepts them and parses the network payload into a
       MqttPacket.

       A MQTT broker implementation, in addition requires to handle permissions and dispatch MQTT messages
       between multiple MqttServer interfaces as well as handle will messages across clients.

       When implementing a MQTT broker, reimplement MqttAutorizer. Then instantiate one or many MqttServer objects
       (depending on the network interfaces to be exposed). Using \l setAutorizer, the custom autorizer is
       registered at the server and the server will ask back to the autorizer for any incoming connection to
       be authorized.
*/

/*!
       \class MqttAuthorizer
       \brief Authorizer base class for authorizing incoming client connections on an \l MqttServer
       \inmodule nymea-mqtt
       \ingroup mqtt

       MqttAuthorizer is the base class for authorization handlers in \l MqttServer interfaces.
       The \l MqttServer will call the authorizer methods on any incoming connect, publish or subscribe
       packets. This can be used to check any user/policy database and authorize/reject such requests
       for particular clients.
*/


#include "mqttserver.h"
#include "mqttserver_p.h"
#include "transports/mqtttcpservertransport.h"
#include "transports/mqttwebsocketservertransport.h"
#include "mqttpacket.h"

#include <QDebug>
#include <QDataStream>
#include <QUuid>
#include <QtGlobal>
#include <QRegularExpression>


Q_LOGGING_CATEGORY(dbgServer, "nymea.mqtt.server")

MqttServerPrivate::MqttServerPrivate(MqttServer *q):
    QObject(q),
    q_ptr(q)
{
    qRegisterMetaType<Mqtt::QoS>();
}

int MqttServerPrivate::listen(MqttServerTransport *transport, const QHostAddress &address, quint16 port)
{
    connect(transport, &MqttServerTransport::clientConnected, this, &MqttServerPrivate::onClientConnected);

    if (!transport->listen(address, port)) {
        qCWarning(dbgServer) << "Error listening on port" << port;
        transport->deleteLater();
        return -1;
    }
    static int addressId = -1;
    servers.insert(++addressId, transport);
    qCDebug(dbgServer) << "nymea MQTT server running on" << address.toString() << ":" << port << "( Address ID" << addressId << ")";
    return addressId;
}

QHash<QString, quint16> MqttServerPrivate::publish(const QString &topic, const QByteArray &payload)
{
    QHash<MqttServerClient*, Mqtt::QoS> receivers;
    foreach (MqttServerClient *c, clientList.keys()) {
        foreach (const MqttSubscription &subscription, clientList.value(c)->subscriptions) {
            if (matchTopic(subscription.topicFilter(), topic)) {
                if (!receivers.contains(c) || receivers.value(c) < subscription.qoS()) {
                    receivers[c] = subscription.qoS();
                }
            }
        }
    }

    QHash<QString, quint16> packets;
    foreach (MqttServerClient *receiver, receivers.keys()) {
        ClientContext *ctx = clientList.value(receiver);
        qCDebug(dbgServer) << "Relaying packet to subscribed client:" << ctx->clientId;
        Mqtt::QoS qos = receivers.value(receiver);
        MqttPacket packet(MqttPacket::TypePublish, qos >= Mqtt::QoS0 ? newPacketId(ctx) : 0, qos);
        packet.setTopic(topic.toUtf8());
        packet.setPayload(payload);
        receiver->write(packet.serialize());
        packets.insert(ctx->clientId, packet.packetId());
        if (packet.qos() == Mqtt::QoS0) {
            QString clientId = ctx->clientId;
            QTimer::singleShot(0, this, [this, clientId, packet](){
                emit q_ptr->published(clientId, packet.packetId(), packet.topic(), packet.payload());
            });
        } else {
            ClientContext *ctx = clientList.value(receiver);
            ctx->unackedPackets.insert(packet.packetId(), packet);
            ctx->unackedPacketList.append(packet.packetId());
        }
    }
    return packets;
}

MqttServer::MqttServer(QObject *parent):
    QObject(parent),
    d_ptr(new MqttServerPrivate(this))
{

}

Mqtt::QoS MqttServer::maximumSubscriptionsQoS() const
{
    return d_ptr->maximumSubscriptionQoS;
}

void MqttServer::setMaximumSubscriptionsQoS(Mqtt::QoS maximumSubscriptionQoS)
{
    d_ptr->maximumSubscriptionQoS = maximumSubscriptionQoS;
}

void MqttServer::setAuthorizer(MqttAuthorizer *authorizer)
{
    d_ptr->authorizer = authorizer;
}

int MqttServer::listen(const QHostAddress &address, quint16 port, const QSslConfiguration &sslConfiguration)
{
    qCDebug(dbgServer) << "Starting nymea MQTT server on TCP";
    MqttServerTransport *transport = new MqttTcpServerTransport(sslConfiguration, this);
    return d_ptr->listen(transport, address, port);
}

int MqttServer::listenWebSocket(const QHostAddress &address, quint16 port, const QSslConfiguration &sslConfiguration)
{
    qCDebug(dbgServer) << "Starting nymea MQTT server on WebSocket";
    MqttServerTransport *transport = new MqttWebSocketServerTransport(sslConfiguration, this);
    return d_ptr->listen(transport, address, port);
}

bool MqttServer::isListening(const QHostAddress &address, quint16 port) const
{
    foreach (MqttServerTransport *transport, d_ptr->servers) {
        if (transport->serverAddress() == address && transport->serverPort() == port && transport->isListening()) {
            return true;
        }
    }
    return false;
}

QList<int> MqttServer::listeningAddressIds() const
{
    return d_ptr->servers.keys();
}

void MqttServer::close(int interfaceId)
{
    if (!d_ptr->servers.contains(interfaceId)) {
        qCWarning(dbgServer) << "No such server address ID" << interfaceId;
        return;
    }
    MqttServerTransport *transport = d_ptr->servers.take(interfaceId);
    while (!d_ptr->clientServerMap.keys(transport).isEmpty()) {
        d_ptr->cleanupClient(d_ptr->clientServerMap.keys(transport).first());
    }
    transport->close();
    transport->deleteLater();
}

QStringList MqttServer::clients() const
{
    QStringList clientIds;
    foreach (ClientContext *ctx, d_ptr->clientList) {
        clientIds << ctx->clientId;
    }
    return clientIds;
}

void MqttServer::disconnectClient(const QString &clientId)
{
    foreach (ClientContext *ctx, d_ptr->clientList) {
        if (ctx->clientId == clientId) {
            d_ptr->cleanupClient(d_ptr->clientList.key(ctx));
            return;
        }
    }
}

QHash<QString, quint16> MqttServer::publish(const QString &topic, const QByteArray &payload)
{
    return d_ptr->publish(topic, payload);
}

void MqttServerPrivate::onClientConnected(MqttServerClient *client)
{
    connect(client, &MqttServerClient::dataAvailable, this, &MqttServerPrivate::onDataAvailable);
    connect(client, &MqttServerClient::disconnected, this, &MqttServerPrivate::onClientDisconnected);

    MqttServerTransport *transport = static_cast<MqttServerTransport*>(sender());

    // Start a 10 second timer to clean up the connection if we don't get data until then.
    QTimer *timeoutTimer = new QTimer(this);
    connect(timeoutTimer, &QTimer::timeout, client, [this, client]() {
        qCWarning(dbgServer) << "A client connected but did not send data in 10 seconds. Dropping connection from" << client->peerAddress();
        pendingConnections.take(client)->deleteLater();
        client->abort();
    });
    timeoutTimer->start(10000);
    clientServerMap.insert(client, transport);
    pendingConnections.insert(client, timeoutTimer);
}

void MqttServerPrivate::onDataAvailable(const QByteArray &data)
{
    MqttServerClient *client = qobject_cast<MqttServerClient*>(sender());

    clientBuffers[client].append(data);

    do {
        MqttPacket packet;
        int ret = packet.parse(clientBuffers[client]);
        if (ret == 0) {
            qCDebug(dbgServer) << "Packet too short... Waiting for more...";
            return;
        }

        // Ok, we've got a full packet (or garbage data). If this client is still pending
        // we can stop the timer, the protocol will take it from here.
        if (pendingConnections.contains(client)) {
            pendingConnections.take(client)->deleteLater();
        }

        if (ret == -1) {
            qCWarning(dbgServer) << "Bad MQTT packet data, Dropping connection" << packet.serialize().toHex();
            cleanupClient(client);
            return;
        }

        clientBuffers[client].remove(0, ret);

        processPacket(packet, client);

    } while (!clientBuffers.value(client).isEmpty());
}

void MqttServerPrivate::onClientDisconnected()
{
    MqttServerClient *client = qobject_cast<MqttServerClient*>(sender());
    cleanupClient(client);
}

void MqttServerPrivate::cleanupClient(MqttServerClient *client)
{
    if (clientBuffers.contains(client)) {
        clientBuffers.remove(client);
    }
    if (clientServerMap.contains(client)) {
        clientServerMap.remove(client);
    }
    if (pendingConnections.contains(client)) {
        delete pendingConnections.take(client);
    }
    if (clientList.contains(client)) {
        ClientContext *ctx = clientList.value(client);
        qCDebug(dbgServer) << "Client" << ctx->clientId << "disconnected.";
        ctx->keepAliveTimer.stop();

        if (!ctx->willTopic.isEmpty()) {
            qCDebug(dbgServer) << "Publishing will message for client" << ctx->clientId << "on topic" << ctx->willTopic << "( Retain:" << ctx->willRetain << ")";
            MqttPacket willPacket(MqttPacket::TypePublish, ctx->willQoS >= Mqtt::QoS1 ? newPacketId(ctx) : 0, ctx->willQoS, ctx->willRetain);
            willPacket.setTopic(ctx->willTopic);
            willPacket.setPayload(ctx->willMessage);
            processPacket(willPacket, client);
        }

        while (!ctx->subscriptions.isEmpty()) {
            emit q_ptr->clientUnsubscribed(ctx->clientId, ctx->subscriptions.takeFirst().topicFilter());
        }

        emit q_ptr->clientDisconnected(ctx->clientId);

        clientList.remove(client);
        delete ctx;
    }

    if (client->isOpen()) {
        client->flush();
        client->close();
    }
    client->deleteLater();
}

void MqttServerPrivate::processPacket(const MqttPacket &packet, MqttServerClient *client)
{
    if (packet.type() == MqttPacket::TypeConnect) {
        if (clientList.contains(client)) {
            ClientContext *ctx = clientList.value(client);
            qCWarning(dbgServer) << "Client" << ctx->clientId << "sends duplicate CONNECT packets. Dropping connection.";
            cleanupClient(client);
            return;
        }

        MqttPacket response(MqttPacket::TypeConnack, packet.packetId());

        if (packet.protocolLevel() != Mqtt::Protocol310 && packet.protocolLevel() != Mqtt::Protocol311) {
            qCWarning(dbgServer) << "This MQTT broker only supports Protocol version 3.1.0 and 3.1.1 but client is" << packet.protocolLevel();
            response.setConnectReturnCode(Mqtt::ConnectReturnCodeUnacceptableProtocolVersion);
            client->write(response.serialize());
            cleanupClient(client);
            return;
        }

        QString clientId = packet.clientId();
        if (clientId.isEmpty()) {
            if (!packet.cleanSession()) {
                qCWarning(dbgServer) << "Empty client id provided but clean session flag not set. Rejecting connection.";
                response.setConnectReturnCode(Mqtt::ConnectReturnCodeIdentifierRejected);
                client->write(response.serialize());
                cleanupClient(client);
                return;
            }
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            clientId = QUuid::createUuid().toString().remove(QRegularExpression("[{}-]*"));
#else
            clientId = QUuid::createUuid().toString().remove(QRegExp("[{}-]*"));
#endif

        }

        if (authorizer) {
            QString username;
            if (packet.connectFlags().testFlag(Mqtt::ConnectFlagUsername)) {
                username = packet.username();
            }
            QString password;
            if (packet.connectFlags().testFlag(Mqtt::ConnectFlagPassword)) {
                password = packet.password();
            }
            MqttServerTransport *transport = clientServerMap.value(client);
            int serverAddressId = servers.key(transport);
            Mqtt::ConnectReturnCode userValidationReturnCode = authorizer->authorizeConnect(serverAddressId, clientId, username, password, client->peerAddress());
            if (userValidationReturnCode != Mqtt::ConnectReturnCodeAccepted) {
                qCWarning(dbgServer).nospace() << "Rejecting connection from " << client->peerAddress().toString() << " due to user validation. (clientId: " << clientId << ", username: " << username << ")";
                response.setConnectReturnCode(userValidationReturnCode);
                client->write(response.serialize());
                cleanupClient(client);
                return;
            }
        }

        ClientContext *ctx = nullptr;

        QList<MqttServerClient*> existingClients = clientList.keys();
        for (int i = 0; i < existingClients.count(); i++) {
            MqttServerClient *existingClient = existingClients.at(i);
            if (clientId == clientList.value(existingClient)->clientId) {
                if (!packet.connectFlags().testFlag(Mqtt::ConnectFlagCleanSession)) {
                    qCDebug(dbgServer).nospace() << clientId << ": Already have a session for this client ID. Taking over existing session.";

                    response.setConnackFlags(Mqtt::ConnackFlagSessionPresent);
                    ctx = clientList.value(existingClient);

                    // remove old client manually, we don't want to clean up the context, nor send any will message or emit disconnected signals
                    clientList.remove(existingClient);
                    clientBuffers.remove(existingClient);
                    existingClient->flush();
                    existingClient->abort();
                    existingClient->deleteLater();
                } else {
                    qCDebug(dbgServer).nospace() << clientId << ": Already have a session for this client ID. Dropping old session.";
                    cleanupClient(existingClient);
                }
                break;
            }
        }

        if (!ctx) {
            if (!packet.connectFlags().testFlag(Mqtt::ConnectFlagCleanSession)) {
                qCWarning(dbgServer).nospace() << clientId << ": Request to take over existing session but we don't have an existing session.";
            }

            ctx = new ClientContext();
            ctx->clientId = clientId;

            connect(&ctx->keepAliveTimer, &QTimer::timeout, this, [this, client](){
                qCWarning(dbgServer) << "Keep alive timeout reached for client:" << clientList.value(client)->clientId;
                cleanupClient(client);
            });
        }

        ctx->keepAlive = packet.keepAlive();
        ctx->version = packet.protocolLevel();


        if (packet.connectFlags().testFlag(Mqtt::ConnectFlagWill)) {
            ctx->willTopic = packet.willTopic();
            ctx->willMessage = packet.willMessage();
            ctx->willRetain = packet.willRetain();
            if (packet.connectFlags().testFlag(Mqtt::ConnectFlagWillQoS2)) {
                ctx->willQoS = Mqtt::QoS2;
            } else if (packet.connectFlags().testFlag(Mqtt::ConnectFlagWillQoS1)) {
                ctx->willQoS = Mqtt::QoS1;
            }
        }
        if (packet.connectFlags().testFlag(Mqtt::ConnectFlagUsername)) {
            ctx->username = packet.username();
        }
        if (packet.connectFlags().testFlag(Mqtt::ConnectFlagPassword)) {
        }

        qCDebug(dbgServer).nospace().noquote()
                << "New MQTT client: \"" << clientId << '\"'
                << ", Protocol: " << packet.protocolName() << " (" << packet.protocolLevel() << ')'
                << ", Flags: " << packet.connectFlags()
                << ", KeepAlive: " << packet.keepAlive()
                << ", Will Topic: \"" << packet.willTopic() << '\"'
                << ", Will Message: \"" << packet.willMessage() << '\"'
                << ", Will Retain: " << packet.willRetain()
                << ", Username: " << packet.username()
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            << ", Password: " << QString(packet.password()).replace(QRegularExpression("."), "*");
#else
            << ", Password: " << QString(packet.password()).replace(QRegExp("."), "*");
#endif


        if (ctx->keepAlive > 0) {
            ctx->keepAliveTimer.start(ctx->keepAlive * 1500);
        }

        clientList.insert(client, ctx);
        response.setConnectReturnCode(Mqtt::ConnectReturnCodeAccepted);
        client->write(response.serialize());
        emit q_ptr->clientConnected(servers.key(clientServerMap.value(client)), ctx->clientId, ctx->username, client->peerAddress());

        foreach (quint16 retryPacketId, ctx->unackedPacketList) {
            qCDebug(dbgServer) << "Resending unacked packet" << retryPacketId << "to" << ctx->clientId;;
            MqttPacket retryPacket = ctx->unackedPackets.value(retryPacketId);
            retryPacket.setDup(true);
            client->write(retryPacket.serialize());
        }
        return;
    }

    if (!clientList.contains(client)) {
        qCWarning(dbgServer) << "Protocol error: Client connection did not send CONNECT yet. Dropping connection.";
        client->close();
        return;
    }

    ClientContext *ctx = clientList.value(client);
    if (ctx->keepAlive > 0) {
        ctx->keepAliveTimer.start();
    }
    emit q_ptr->clientAlive(ctx->clientId);

    if (packet.type() == MqttPacket::TypePublish) {
        qCDebug(dbgServer).nospace() << "Publish received from client " << ctx->clientId << ": Topic: " << packet.topic() << ", Payload: " << packet.payload() << " (Packet ID: " << packet.packetId() << ", DUP: " << packet.dup() << ", QoS: " << packet.qos() << ", Retain: " << packet.retain() << ')';
        switch (packet.qos()) {
        case Mqtt::QoS0:
            break;
        case Mqtt::QoS1: {
            MqttPacket response(MqttPacket::TypePuback, packet.packetId());
            client->write(response.serialize());
            break;
        }
        case Mqtt::QoS2: {
            if (packet.dup() && ctx->unackedPacketList.contains(packet.packetId())) {
                // We received this message before but the client keeps on trying... Just send a PUBREC and stop processing
                client->write(ctx->unackedPackets.value(packet.packetId()).serialize());
                return;
            } else if (ctx->unackedPacketList.contains(packet.packetId())) {
                // Hmm... Client says this is a new packet, but the ID is not released yet! Drop client connection.
                qCWarning(dbgServer()).nospace() << "Received a bad packet from \"" << ctx->clientId << "\". DUP is not set but packet ID is already used and not released. Dropping client connection.";
                cleanupClient(client);
                return;
            }
            // Ok, a new packet, ack it with a PUBREC and store the number
            MqttPacket response(MqttPacket::TypePubrec, packet.packetId());
            ctx->unackedPackets.insert(response.packetId(), response);
            ctx->unackedPacketList.append(packet.packetId());
            client->write(response.serialize());
            break;
        }
        }
        if (packet.retain()) {
            if (packet.payload().isEmpty()) {
                qCDebug(dbgServer) << "Clearing retained messages for topic" << packet.topic();
                retainedMessages.remove(packet.topic());
            } else {
                if (packet.qos() == Mqtt::QoS0) {
                    qCDebug(dbgServer) << "Clearing retained messages for topic" << packet.topic();
                    retainedMessages.remove(packet.topic());
                }
                qCDebug(dbgServer) << "Adding retained message for topic" << packet.topic();
                retainedMessages[packet.topic()].append(packet);
            }
        }

        if (authorizer && !authorizer->authorizePublish(servers.key(clientServerMap.value(client)), ctx->clientId, packet.topic())) {
            qCDebug(dbgServer) << "Client not authorized to publish to this topic. Discarding packet";
            return;
        }

        emit q_ptr->publishReceived(ctx->clientId, packet.packetId(), packet.topic(), packet.payload());
        publish(packet.topic(), packet.payload());

        return;
    }
    if (packet.type() == MqttPacket::TypePuback) {
        ctx->unackedPacketList.removeAll(packet.packetId());
        MqttPacket publishedPacket = ctx->unackedPackets.take(packet.packetId());
        emit q_ptr->published(ctx->clientId, packet.packetId(), publishedPacket.topic(), publishedPacket.payload());
        return;
    }
    if (packet.type() == MqttPacket::TypePubrec) {
        MqttPacket publishedPacket = ctx->unackedPackets.take(packet.packetId());
        emit q_ptr->published(ctx->clientId, packet.packetId(), publishedPacket.topic(), publishedPacket.payload());
        MqttPacket pubrel(MqttPacket::TypePubrel, packet.packetId());
        ctx->unackedPackets.insert(packet.packetId(), pubrel);
        client->write(pubrel.serialize());
        return;
    }
    if (packet.type() == MqttPacket::TypePubrel) {
        ctx->unackedPackets.remove(packet.packetId());
        ctx->unackedPacketList.removeAll(packet.packetId());
        MqttPacket response(MqttPacket::TypePubcomp, packet.packetId());
        client->write(response.serialize());
        return;
    }
    if (packet.type() == MqttPacket::TypePubcomp) {
        ctx->unackedPackets.remove(packet.packetId());
        ctx->unackedPacketList.removeAll(packet.packetId());
        return;
    }
    if (packet.type() == MqttPacket::TypeSubscribe) {
//        qCDebug(dbgServer).nospace() << ctx->clientId ": Subscribe packet received.";
        MqttPacket response(MqttPacket::TypeSuback, packet.packetId());
        QByteArray payload;
        MqttSubscriptions effectiveSubscriptions;
        foreach (MqttSubscription subscription, packet.subscriptions()) {
            if (authorizer && !authorizer->authorizeSubscribe(servers.key(clientServerMap.value(client)), ctx->clientId, subscription.topicFilter())) {
                qCWarning(dbgServer).nospace().noquote() << "Subscription topic filter not allowed for client \"" << ctx->clientId << "\": \"" << subscription.topicFilter() << '\"';
                response.addSubscribeReturnCode(Mqtt::SubscribeReturnCodeFailure);
                continue;
            }
            if (!validateTopicFilter(subscription.topicFilter())) {
                qCWarning(dbgServer).nospace() << "Subscription topic filter not valid for client \"" << ctx->clientId << "\": " << subscription.topicFilter();
                response.addSubscribeReturnCode(Mqtt::SubscribeReturnCodeFailure);
                continue;
            }
            subscription.setQoS(qMin(subscription.qoS(), maximumSubscriptionQoS));
            bool updated = false;
            for (int i = 0; i < ctx->subscriptions.count(); i++) {
                if (ctx->subscriptions.at(i).topicFilter() == subscription.topicFilter()) {
                    qCDebug(dbgServer).noquote().nospace() << "Client \"" << ctx->clientId << "\" subscribed with a duplicate topic filter. Replacing subcription with new QoS" << subscription.qoS();
                    ctx->subscriptions.replace(i, subscription);
                    updated = true;
                }
            }
            if (!updated) {
                ctx->subscriptions.append(subscription);
            }
            qCDebug(dbgServer).noquote().nospace() << "Subscribed client \"" << ctx->clientId << "\" to topic filter: \"" << subscription.topicFilter() << "\" with QoS " << subscription.qoS();
            effectiveSubscriptions << subscription;
            emit q_ptr->clientSubscribed(ctx->clientId, subscription.topicFilter(), subscription.qoS());
            switch (subscription.qoS()) {
            case Mqtt::QoS0:
                response.addSubscribeReturnCode(Mqtt::SubscribeReturnCodeSuccessQoS0);
                break;
            case Mqtt::QoS1:
                response.addSubscribeReturnCode(Mqtt::SubscribeReturnCodeSuccessQoS1);
                break;
            case Mqtt::QoS2:
                response.addSubscribeReturnCode(Mqtt::SubscribeReturnCodeSuccessQoS2);
                break;
            }
        }
        client->write(response.serialize());

        // Deliver any retained messages for this topic
        foreach (MqttSubscription subscription, effectiveSubscriptions) {
            foreach (const QString &topic, retainedMessages.keys()) {
                if (matchTopic(subscription.topicFilter(), topic)) {
                    foreach (MqttPacket packet, retainedMessages.value(topic)) {
                        packet.setRetain(true);
                        client->write(packet.serialize());
                    }
                }
            }
        }
        return;
    }
    if (packet.type() == MqttPacket::TypeUnsubscribe) {
        MqttSubscriptions newSubscriptions;
        foreach (const MqttSubscription &existingSubscription, ctx->subscriptions) {
            bool matching = false;
            foreach (const MqttSubscription &unsub, packet.subscriptions()) {
                if (existingSubscription.topicFilter() == unsub.topicFilter()) {
                    qCDebug(dbgServer) << "Unsubscribing client" << ctx->clientId << "from" << unsub.topicFilter();
                    emit q_ptr->clientUnsubscribed(ctx->clientId, unsub.topicFilter());
                    matching = true;
                    break;
                }
            }
            if (!matching) {
                newSubscriptions.append(existingSubscription);
            }
        }
        ctx->subscriptions = newSubscriptions;
        MqttPacket response(MqttPacket::TypeUnsuback, packet.packetId());
        client->write(response.serialize());
        return;
    }
    if (packet.type() == MqttPacket::TypePingreq) {
//        qCDebug(dbgServer).nospace() << ctx->clientId << ": Pingreq received";
        MqttPacket response(MqttPacket::TypePingresp, packet.packetId());
        client->write(response.serialize());
        return;
    }
    if (packet.type() == MqttPacket::TypeDisconnect) {
        ctx->willMessage.clear();
        ctx->willTopic.clear();
        return;
    }
    qCWarning(dbgServer).nospace().noquote() << "Unknown packet received from client \"" << ctx->clientId << "\": " << QString::number(packet.type(), 16);
    Q_ASSERT(false);
    cleanupClient(client);

}

bool MqttServerPrivate::validateTopicFilter(const QString &topicFilter)
{
    if (topicFilter.length() < 1) {
        return false;
    }
    QStringList parts = topicFilter.split('/');
    for (int i = 0; i < parts.count(); i++) {
        const QString &part = parts.at(i);
        if (part.contains(QStringLiteral("#")) && (part != QStringLiteral("#") || i != parts.count() - 1)) {
            return false;
        }
        if (part.contains(QStringLiteral("+")) && part != QStringLiteral("+")) {
            return false;
        }
    }

    return true;
}

bool MqttServerPrivate::matchTopic(const QString &topicFilter, const QString &topic)
{
    if (topic.startsWith('$')) {
        return false;
    }

    QStringList filterParts = topicFilter.split('/');
    QStringList topicParts = topic.split('/');

    if (topicParts.count() < filterParts.count() - 1) {
        return false;
    }

    for (int i = 0; i < filterParts.count(); i++) {
        if (filterParts.at(i) == QStringLiteral("+")) {
            continue;
        }
        if (filterParts.at(i) == QStringLiteral("#")) {
            continue;
        }
        if (topicParts.length() <= i) {
            return false;
        }
        if (topicParts.at(i) == filterParts.at(i)) {
            continue;
        }
        return false;
    }

    return filterParts.count() == topicParts.count() || topicFilter.endsWith('#');
}

quint16 MqttServerPrivate::newPacketId(ClientContext *ctx)
{
    static quint16 packetId = 0;
    do {
        packetId++;
    } while(ctx->unackedPacketList.contains(packetId));
    return packetId;
}

