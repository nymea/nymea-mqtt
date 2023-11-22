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

#include "mqttserver.h"
#include "mqttclient.h"
#include "mqttclient_p.h"

#include <QTest>
#include <QSignalSpy>

#include "mqtttests.h"

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))

MqttClient *MqttTests::connectAndWait(const QString &clientId, bool cleanSession, quint16 keepAlive, const QString &willTopic, const QString &willMessage, Mqtt::QoS willQoS, bool willRetain)
{
    QPair<MqttClient*, QSignalSpy*> result = connectToServer(clientId, cleanSession, keepAlive, willTopic, willMessage, willQoS, willRetain);
    if (result.second->count() == 0) {
        result.second->wait();
    }

    if (result.second->count() == 0) {
        qWarning() << "WARNING: Client didn't emit connected";
    }
    delete result.second;
    return result.first;
}

QPair<MqttClient*, QSignalSpy*> MqttTests::connectToServer(const QString &clientId, bool cleanSession, quint16 keepAlive, const QString &willTopic, const QString &willMessage, Mqtt::QoS willQoS, bool willRetain)
{
    MqttClient* client = new MqttClient(clientId, keepAlive, willTopic, willMessage.toUtf8(), willQoS, willRetain, this);
    client->setAutoReconnect(false);

    m_clients.append(client);

    QSignalSpy *spy = new QSignalSpy(client, &MqttClient::connected);

    connectClientToServer(client, cleanSession);

    return QPair<MqttClient*, QSignalSpy*>(client, spy);
}

void MqttTests::disconnectAndWait(MqttClient* client)
{
    QSignalSpy disconnectedSpy(client, &MqttClient::disconnected);
    client->disconnectFromHost();
    if (disconnectedSpy.count() == 0) {
        disconnectedSpy.wait();
    }
}

bool MqttTests::subscribeAndWait(MqttClient* client, const QString &topic, Mqtt::QoS qos)
{
    QSignalSpy subscribedSpy(client, &MqttClient::subscribeResult);
    quint16 packetId = client->subscribe(topic, qos);
    if (subscribedSpy.count() == 0) {
        subscribedSpy.wait();
    }
    Mqtt::SubscribeReturnCode expectedSubscribeReturnCode = qos == Mqtt::QoS0 ? Mqtt::SubscribeReturnCodeSuccessQoS0 : qos == Mqtt::QoS1 ? Mqtt::SubscribeReturnCodeSuccessQoS1 : Mqtt::SubscribeReturnCodeSuccessQoS2;
    return subscribedSpy.count() == 1 && subscribedSpy.first().at(0).toInt() == packetId && subscribedSpy.first().at(1).value<Mqtt::SubscribeReturnCodes>().first() == expectedSubscribeReturnCode;
}

void MqttTests::initTestCase()
{
//    QLoggingCategory::setFilterRules("nymea.mqtt.protocol.debug=false");

    m_server = new MqttServer(this);

    m_serverId = startServer(m_server);

    QVERIFY2(m_serverId >= 0, "Failed to register server. Tests won't work.");
}

void MqttTests::cleanup()
{
    while (!m_clients.isEmpty()) {
        MqttClient *client = m_clients.takeFirst();
        client->disconnectFromHost();
        client->deleteLater();
    }
    QTRY_COMPARE(m_server->clients().count(), 0);
}

void MqttTests::cleanupTestCase()
{
    m_server->close(m_serverId);
    delete m_server;
}

void MqttTests::connectAndDisconnect()
{
    QSignalSpy serverSpy(m_server, &MqttServer::clientConnected);

    QString clientId = "connectAndDisconnect-client";
    QPair<MqttClient*, QSignalSpy*> result = connectToServer(clientId);
    MqttClient* client = result.first;
    connect(client, &MqttClient::connected, this, [client](Mqtt::ConnectReturnCode connectReturnCode, Mqtt::ConnackFlags connackFlags){
        QVERIFY2(client->isConnected(), "MqttClient::isConnected not returning true in connected signal()");
        QCOMPARE(connectReturnCode, Mqtt::ConnectReturnCodeAccepted);
        QCOMPARE(connackFlags, Mqtt::ConnackFlagNone);
    });
    if (result.second->count() == 0) {
        result.second->wait();
    }

    QVERIFY2(serverSpy.count() == 1, "Server didn't emit clientConnected");
    QVERIFY2(serverSpy.at(0).at(1) == clientId, "ClientId not matching on server side.");

    QSignalSpy serverSpyDisconnect(m_server, &MqttServer::clientDisconnected);
    QSignalSpy clientSpy(client, &MqttClient::disconnected);
    client->disconnectFromHost();
    QTRY_VERIFY2(clientSpy.count() == 1, "client didn't emit disconnected");
    QTRY_VERIFY2(serverSpyDisconnect.count() == 1, "Server didn't emit clientDisconnected");
    QVERIFY2(serverSpyDisconnect.at(0).first() == clientId, "ClientId not matching on server side.");
}

void MqttTests::keepAliveTimesOut()
{
    QSignalSpy keepAliveSpy(m_server, &MqttServer::clientAlive);
    MqttClient *client = connectAndWait("keepAlive1sec-client", true, 1);
    client->setAutoReconnect(false);
    QTest::qWait(2000);
    qDebug() << "Received" << keepAliveSpy.count() << "keep alive messages";
    QVERIFY2(client->isConnected(), "Client connection dropped");
    QVERIFY2(keepAliveSpy.count() > 0, "Keep alive not received");

    client->disconnectFromHost();

    keepAliveSpy.clear();

    client = connectAndWait("timeout1sec-client", true, 1);
    client->setAutoReconnect(false);
    client->d_ptr->keepAliveTimer.stop(); // disable the keepalive timer
    QTest::qWait(2000);
    qDebug() << "Received" << keepAliveSpy.count() << "keep alive messages";
    QVERIFY2(!client->isConnected(), "Client connection still alive but it should have been dropped");
}

void MqttTests::subscribeAndPublish_data()
{
    QTest::addColumn<Mqtt::QoS>("qosClient1");
    QTest::addColumn<Mqtt::QoS>("qosClient2");

    QList<QList<Mqtt::QoS> > rows;
    rows.append({Mqtt::QoS0, Mqtt::QoS0});
    rows.append({Mqtt::QoS0, Mqtt::QoS1});
    rows.append({Mqtt::QoS0, Mqtt::QoS2});
    rows.append({Mqtt::QoS1, Mqtt::QoS0});
    rows.append({Mqtt::QoS1, Mqtt::QoS1});
    rows.append({Mqtt::QoS1, Mqtt::QoS2});
    rows.append({Mqtt::QoS2, Mqtt::QoS0});
    rows.append({Mqtt::QoS2, Mqtt::QoS1});
    rows.append({Mqtt::QoS2, Mqtt::QoS2});

    foreach (const QList<Mqtt::QoS> &row, rows) {
        QTest::newRow(QString("Subscribe QoS%1 -> Publish QoS%2").arg(row.at(0)).arg(row.at(1)).toUtf8().data()) << row.at(0) << row.at(1);
    }
}

void MqttTests::subscribeAndPublish()
{
    QFETCH(Mqtt::QoS, qosClient1);
    QFETCH(Mqtt::QoS, qosClient2);

    QString clientId1 = QString("subQoS%1-client").arg(qosClient1);
    MqttClient *client1 = connectAndWait(clientId1);
    QString clientId2 = QString("pubQoS%1-client").arg(qosClient2);
    MqttClient *client2 = connectAndWait(clientId2);

    QSignalSpy serverSubscribeSpy(m_server, &MqttServer::clientSubscribed);
    QSignalSpy clientSubscribeSpy(client1, &MqttClient::subscribeResult);

    quint16 packetId = client1->subscribe("#", qosClient1);

    QTRY_VERIFY2(serverSubscribeSpy.count() == 1, "Server did not emit clientSubscribed");
    QVERIFY2(serverSubscribeSpy.first().first().toString() == clientId1, "Client Id not matching");
    QVERIFY2(serverSubscribeSpy.first().at(1).toString() == "#", "Topic not matching");
    QVERIFY2(serverSubscribeSpy.first().at(2).toInt() == qosClient1, "QoS not matching");

    QTRY_VERIFY2(clientSubscribeSpy.count() == 1, "Client did not emit subscribed");
    QVERIFY2(clientSubscribeSpy.first().first().toInt() == packetId, "Packet ID not matching");
    QVERIFY2(clientSubscribeSpy.first().at(1).value<Mqtt::SubscribeReturnCodes>().count() == 1, "Subscribe return code count not matching");

    QSignalSpy serverPublishReceivedSpy(m_server, &MqttServer::publishReceived);
    QSignalSpy serverPublishedSpy(m_server, &MqttServer::published);
    QSignalSpy client1PublishReceivedSpy(client1, &MqttClient::publishReceived);
    QSignalSpy client2PublishedSpy(client2, &MqttClient::published);

    packetId = client2->publish("/testtopic/", "Hello world", qosClient2);

    QTRY_VERIFY2(serverPublishReceivedSpy.count() == 1, "Server did not emit publishReceived");
    QVERIFY2(serverPublishReceivedSpy.first().at(0).toString() == clientId2, "Server did emit publishReceived signal but client ID is not matching");
    QVERIFY2(serverPublishReceivedSpy.first().at(1).toInt() == packetId, QString("Server did emit publishReceived signal but Packet ID is not matching:\nActual: %1\nExpected: %2").arg(serverPublishReceivedSpy.first().at(1).toInt()).arg(packetId).toUtf8().data());
    QVERIFY2(serverPublishReceivedSpy.first().at(2).toString() == "/testtopic/", "Server did emit publishReceived signal but topic is not matching");
    QVERIFY2(serverPublishReceivedSpy.first().at(3).toByteArray() == QByteArray("Hello world"), "Server did emit publishReceived signal but payload is not matching");

    QTRY_VERIFY2(serverPublishedSpy.count() == 1, "Server did not emit published");
    QVERIFY2(serverPublishedSpy.first().at(0).toString() == clientId1, "Server did emit published signal but client ID is not matching");

    QTRY_VERIFY2(client1PublishReceivedSpy.count() == 1, "Subscribing client did not emit publishReceived signal");
    QVERIFY2(client1PublishReceivedSpy.first().at(0).toString() == "/testtopic/", "Subscribing client did emit publishReceived signal but topic is not matching");
    QVERIFY2(client1PublishReceivedSpy.first().at(1).toByteArray() == QByteArray("Hello world"), "Subscribing client did emit publishReceived signal but payload is not matching");

    QTRY_VERIFY2(client2PublishedSpy.count() == 1, "Publishing client did not emit published signal");
    QVERIFY2(client2PublishedSpy.first().first().toInt() == packetId, "Publishing client did emit published signal but packet ID not matching");

}

void MqttTests::willIsSentOnClientDisappearing()
{
    MqttClient *client1 = connectAndWait("subWill-client");
    MqttClient *client2 = connectAndWait("pubWill-client", true, 300, "/testtopic", "Bye bye");

    QSignalSpy publishSpy(client1, &MqttClient::publishReceived);

    QVERIFY(subscribeAndWait(client1, "#"));

    client2->d_ptr->transport->abort();

    QTRY_VERIFY2(publishSpy.count() == 1, "Will has not been sent");
    QVERIFY2(publishSpy.first().at(0).toString() == "/testtopic", "Will topic not matching");
    QVERIFY2(publishSpy.first().at(1).toByteArray() == QByteArray("Bye bye"), "Will message not matching");
}

void MqttTests::willIsNotSentOnClientDisconnecting()
{
    MqttClient *client1 = connectAndWait("subWill-client");
    MqttClient *client2 = connectAndWait("pubWill-client", true, 300, "/testtopic", "Bye bye");

    QSignalSpy subscribeSpy(client1, &MqttClient::subscribeResult);
    QSignalSpy publishSpy(client1, &MqttClient::publishReceived);

    client1->subscribe("#");
    subscribeSpy.wait();

    client2->disconnectFromHost();

    publishSpy.wait(200);
    QVERIFY2(publishSpy.count() == 0, "Will has been sent but it should not have been");
}

void MqttTests::testWillRetain()
{
    MqttClient *client1 = connectAndWait("subWill-client");
    MqttClient *client2 = connectAndWait("pubWill-client", true, 300, "/testtopic", "Bye bye", Mqtt::QoS1, true);

    QSignalSpy subscribeSpy(client1, &MqttClient::subscribeResult);
    QSignalSpy publishSpy(client1, &MqttClient::publishReceived);

    client1->subscribe("#");
    subscribeSpy.wait();

    client2->setAutoReconnect(false);
    client2->d_ptr->transport->abort();

    QTRY_VERIFY2(publishSpy.count() == 1, "Will has not been sent");
    QVERIFY2(publishSpy.first().at(0).toString() == "/testtopic", QString("Will topic not matching: %1").arg(publishSpy.first().at(0).toString()).toUtf8().data());
    QVERIFY2(publishSpy.first().at(1).toByteArray() == QByteArray("Bye bye"), "Will message not matching");
    QVERIFY2(publishSpy.first().at(2).toBool() == false, "Retain flag not matching");

    MqttClient *client3 = connectAndWait("subWill-client2");
    QSignalSpy retainedWillSpy(client3, &MqttClient::publishReceived);

    client3->subscribe("#");
    QTRY_VERIFY2(retainedWillSpy.count() == 1, "Retained Will has not been sent");
    QVERIFY2(retainedWillSpy.first().at(0).toString() == "/testtopic", "Will topic not matching");
    QVERIFY2(retainedWillSpy.first().at(1).toByteArray() == QByteArray("Bye bye"), "Will message not matching");
    QVERIFY2(retainedWillSpy.first().at(2).toBool() == true, "Retain flag not matching");

    // Clear retain on /testtopic
    QSignalSpy clearRetainSpy(client3, &MqttClient::published);
    client3->publish("/testtopic", QByteArray(), Mqtt::QoS1, true);
    QTRY_VERIFY2(clearRetainSpy.count() == 1, "Clearing retain message did not succeed");
}

void MqttTests::testAutoReconnect()
{
    MqttClient *client1 = connectAndWait("client1");
    client1->setAutoReconnect(true);

    QSignalSpy disconnectedSpy(client1, &MqttClient::disconnected);
    QSignalSpy connectedSpy(client1, &MqttClient::connected);

    client1->d_ptr->transport->abort();

    QTRY_VERIFY2(disconnectedSpy.count() == 1, "client did not emit disconnected");
    QTRY_VERIFY2(connectedSpy.count() == 1, "client did not emit connected");
}

void MqttTests::testQoS1Retransmissions()
{
    QSignalSpy serverSpy(m_server, &MqttServer::publishReceived);

    MqttClient *client = connectAndWait("client1");
    client->setAutoReconnect(true);

    // publish a packet, flush the pipe and immediately drop the connection before we have a chance to receive the PUBACK
    int packetId = client->publish("/testtopic", "Hello world", Mqtt::QoS1);
    client->d_ptr->transport->flush();
    QSignalSpy connectedSpy(client, &MqttClient::connected);
    client->d_ptr->transport->abort();

    // Wait for it to reconnect, it should then republish the packet
    connectedSpy.wait();

    QTRY_VERIFY2(serverSpy.count() == 2, "Server didn't receive the publication twice but it should have");
    QCOMPARE(serverSpy.at(0).at(0).toString(), QString("client1"));
    QCOMPARE(serverSpy.at(0).at(1).toInt(), packetId);
    QCOMPARE(serverSpy.at(0).at(2).toString(), QString("/testtopic"));
    QCOMPARE(serverSpy.at(0).at(3).toByteArray(), QByteArray("Hello world"));

    QCOMPARE(serverSpy.at(1).at(0).toString(), QString("client1"));
    QCOMPARE(serverSpy.at(1).at(1).toInt(), packetId);
    QCOMPARE(serverSpy.at(1).at(2).toString(), QString("/testtopic"));
    QCOMPARE(serverSpy.at(1).at(3).toByteArray(), QByteArray("Hello world"));
}

void MqttTests::testMultiSubscription()
{
    MqttClient *client = connectAndWait("subscription-topics");
    QSignalSpy subscribedSpy(client, &MqttClient::subscribeResult);

    MqttSubscriptions subscriptions = { MqttSubscription("topic1"), MqttSubscription("topic2") , MqttSubscription("#invalid") };
    Mqtt::SubscribeReturnCodes subscriptionReturnCodes = { Mqtt::SubscribeReturnCodeSuccessQoS0, Mqtt::SubscribeReturnCodeSuccessQoS0, Mqtt::SubscribeReturnCodeFailure};

    client->subscribe(subscriptions);
    QTRY_VERIFY2(subscribedSpy.count() == 1, "Subscribed signal not received");

    Mqtt::SubscribeReturnCodes retCodes = subscribedSpy.first().at(1).value<Mqtt::SubscribeReturnCodes>();
    QCOMPARE(retCodes, subscriptionReturnCodes);
}

void MqttTests::testSubscriptionTopicFilters_data()
{
    QTest::addColumn<QString>("topicFilter");
    QTest::addColumn<Mqtt::SubscribeReturnCode>("subscriptionReturnCode");

    QTest::newRow("a") << "a" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("/") << "/" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("/a") << "/a" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("//") << "//" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("/a/") << "/a/" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("/a/b") << "/a/b" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("//b") << "//b" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("#") << "#" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("a/#") << "a/#" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("a/b#") << "a/b#" << Mqtt::SubscribeReturnCodeFailure;
    QTest::newRow("a/b/#/c") << "a/b/#/c" << Mqtt::SubscribeReturnCodeFailure;
    QTest::newRow("+") << "+" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("+/a/#") << "+/a/#" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("a+") << "a+" << Mqtt::SubscribeReturnCodeFailure;
    QTest::newRow("a/+/b") << "a/+/b" << Mqtt::SubscribeReturnCodeSuccessQoS0;
    QTest::newRow("+/a/#") << "+/a/#" << Mqtt::SubscribeReturnCodeSuccessQoS0;
}

void MqttTests::testSubscriptionTopicFilters()
{
    QFETCH(QString, topicFilter);
    QFETCH(Mqtt::SubscribeReturnCode, subscriptionReturnCode);

    MqttClient *client = connectAndWait("subscription-topics");
    QSignalSpy subscribedSpy(client, &MqttClient::subscribeResult);
    client->subscribe(topicFilter);
    QTRY_VERIFY2(subscribedSpy.count() == 1, "Subscribed signal not received");

    Mqtt::SubscribeReturnCodes retCodes = subscribedSpy.first().at(1).value<Mqtt::SubscribeReturnCodes>();
    QCOMPARE(retCodes.first(), subscriptionReturnCode);
}

void MqttTests::testSubscriptionTopicMatching_data()
{
    QTest::addColumn<QString>("topicFilter");
    QTest::addColumn<QString>("topic");
    QTest::addColumn<int>("receivedPublishMessageCount");

    QList<QStringList> rows;
    rows.append({ "a", "a", "1" });
    rows.append({ "a", "b", "0" });
    rows.append({ "/", "/" , "1" });
    rows.append({ "/", "/a" , "0" });
    rows.append({ "#", "a", "1" });
    rows.append({ "#", "a/b", "1" });
    rows.append({ "+", "a", "1" });
    rows.append({ "+", "a/", "0" });
    rows.append({ "+", "/a" , "0" });
    rows.append({ "+", "a/b", "0" });

    rows.append({ "/#", "/" , "1" });
    rows.append({ "/+", "/a" , "1" });
    rows.append({ "/a", "/a" , "1" });
    rows.append({ "/a", "/a" , "1" });
    rows.append({ "a/+", "a", "0" });
    rows.append({ "a/+", "a/", "1" });
    rows.append({ "a/+", "a/b", "1" });
    rows.append({ "a/+", "a/b/c", "0" });
    rows.append({ "+/+", "/a" , "1" });
    rows.append({ "+/+", "/a" , "1" });
    rows.append({ "+/+", "a/" , "1" });
    rows.append({ "a/#", "a/b", "1" });
    rows.append({ "a/#", "a", "1" });
    rows.append({ "a/#", "/", "0" });
    rows.append({ "a/#", "a/b/c", "1" });
    rows.append({ "a/#", "b/c", "0" });
    rows.append({ "a//", "a//", "1" });
    rows.append({ "a//#", "a//b", "1" });
    rows.append({ "a/b/+", "a/b/c", "1" });
    rows.append({ "a/b/+", "a/b/d", "1" });
    rows.append({ "a/b/+", "a/b/c/d", "0" });
    rows.append({ "+/a/#", "a/a/b", "1" });
    rows.append({ "+/a/#", "a/a/b/c", "1" });
    rows.append({ "+/a/#", "d/a/b/c", "1" });
    rows.append({ "+/a/#", "a/b/c/d", "0" });
    rows.append({ "a/b/#", "a/b/c", "1" });
    rows.append({ "a//+/", "a//b/", "1" });
    rows.append({ "a//+/", "a///", "1" });
    rows.append({ "a//+/#", "a//b/c", "1" });
    rows.append({ "a//+/#", "a/b/c/d", "0" });
    rows.append({ "a/b/c", "a/b", "0"});

    rows.append({ "$SYS/", "$SYS/", "0" });
    rows.append({ "#", "$SYS/", "0" });
    rows.append({ "+/", "$SYS/", "0" });

    foreach (const QStringList &row, rows) {
        QTest::newRow(QString("%1, %2").arg(row.at(0), row.at(1)).toUtf8().data()) << row.at(0) << row.at(1) << row.at(2).toInt();
    }
}

void MqttTests::testSubscriptionTopicMatching()
{
    QFETCH(QString, topicFilter);
    QFETCH(QString, topic);
    QFETCH(int, receivedPublishMessageCount);

    MqttClient *publisher = connectAndWait("publisher");
    MqttClient *subscriber = connectAndWait("subscriber");

    QSignalSpy subscribedSpy(subscriber, &MqttClient::subscribeResult);
    QSignalSpy publishReceivedSpy(subscriber, &MqttClient::publishReceived);
    QSignalSpy publishedSpy(publisher, &MqttClient::published);

    subscriber->subscribe(topicFilter);
    QTRY_VERIFY2(subscribedSpy.count() == 1, "Subscribed signal not received");

    publisher->publish(topic, "testpayload");
    QTRY_VERIFY2(publishedSpy.count() == 1, "Published signal not received");

    if (receivedPublishMessageCount == 0) {
        // Give it some time to wait for a publishReceived (It should not show up)
        QTest::qWait(500);
    } else if (publishReceivedSpy.count() == 0) {
        publishReceivedSpy.wait();
    }
    QVERIFY2(publishReceivedSpy.count() == receivedPublishMessageCount, QString("PublishReceived signal not received the expected amount of time.\nActual: %1\nExpected: %2").arg(publishReceivedSpy.count()).arg(receivedPublishMessageCount).toUtf8().data());
}

void MqttTests::testSessionManagementDropOldSession()
{
    MqttClient *client1Session1 = connectAndWait("client1");
    client1Session1->setAutoReconnect(false);

    QSignalSpy subscribeSpy(client1Session1, &MqttClient::subscribeResult);
    client1Session1->subscribe("/testtopic");
    QTRY_VERIFY(subscribeSpy.count() == 1);

    QSignalSpy disconnectedSpy(client1Session1, &MqttClient::disconnected);

    QPair<MqttClient*, QSignalSpy*> client1Session2 = connectToServer("client1");
    if (client1Session2.second->count() == 0) {
        client1Session2.second->wait();
    }
    QVERIFY2(!client1Session2.second->first().at(0).value<Mqtt::ConnackFlags>().testFlag(Mqtt::ConnackFlagSessionPresent), "Session present flag is set while it should not be.");

    QTRY_VERIFY2(disconnectedSpy.count() == 1, "First instance didn't get disconnected when new instance connected.");


    // Now connect with another client and post to testtopic. Client 1 should not get it because he didn't resume the session and didn't resubscribe
    QSignalSpy client1PublishReceivedSpy(client1Session2.first, &MqttClient::publishReceived);

    MqttClient *client2 = connectAndWait("client2");

    client2->publish("/testtopic", "Hello world");

    QTest::qWait(500);

    QVERIFY2(client1PublishReceivedSpy.count() == 0, "Client 1 did receive the publish but it should not have.");
}

void MqttTests::testSessionManagementResumeOldSession()
{
    MqttClient *client1Session1 = connectAndWait("client1");
    client1Session1->setAutoReconnect(false);

    QSignalSpy subscribeSpy(client1Session1, &MqttClient::subscribeResult);
    client1Session1->subscribe("/testtopic");
    QTRY_VERIFY(subscribeSpy.count() == 1);

    QSignalSpy disconnectedSpy(client1Session1, &MqttClient::disconnected);

    QPair<MqttClient*, QSignalSpy*> client1Session2 = connectToServer("client1", false);
    if (client1Session2.second->count() == 0) {
        client1Session2.second->wait();
    }
    QVERIFY2(client1Session2.second->first().at(0).value<Mqtt::ConnectReturnCode>() == Mqtt::ConnectReturnCodeAccepted, "Session hasn't been accepted.");
    QVERIFY2(client1Session2.second->first().at(1).value<Mqtt::ConnackFlags>().testFlag(Mqtt::ConnackFlagSessionPresent), "Session present flag is not set while it should be.");

    QTRY_VERIFY2(disconnectedSpy.count() == 1, "First instance didn't get disconnected when new instance connected.");


    // Now connect with another client and post to testtopic. Client 1 should not get it because he didn't resume the session and didn't resubscribe
    QSignalSpy client1PublishReceivedSpy(client1Session2.first, &MqttClient::publishReceived);

    MqttClient *client2 = connectAndWait("client2");

    client2->publish("/testtopic", "Hello world");

    QTRY_VERIFY2(client1PublishReceivedSpy.count() == 1, "Client 1 did not receive the publish but it should have.");
}

void MqttTests::testSessionManagementFailResumeOldSession()
{
    // try to resume non existing session
    QPair<MqttClient*, QSignalSpy*> client = connectToServer("client1", false);
    if (client.second->count() == 0) {
        client.second->wait();
    }
    QVERIFY2(!client.second->first().at(0).value<Mqtt::ConnackFlags>().testFlag(Mqtt::ConnackFlagSessionPresent), "Session present flag is set while it should not be.");
}

void MqttTests::testQoS1PublishToServerIsAckedOnSessionResume()
{
    MqttClient *client = connectAndWait("client1", true);
    client->setAutoReconnect(true);

    QSignalSpy reconnectedSpy(client, &MqttClient::connected);

    QSignalSpy publishedSpy(client, &MqttClient::published);
    client->publish("/testtopic", "Hello world", Mqtt::QoS1);
    client->d_ptr->transport->flush();
    client->d_ptr->transport->abort();

    QVERIFY2(publishedSpy.count() == 0, "Should not have received the PUBACK yet... Test is bad.");

    QTRY_VERIFY2(reconnectedSpy.count() == 1, "client didn't reconnect");

    QTRY_VERIFY2(publishedSpy.count() == 1, "Published signal not emitted after reconnect");

}

void MqttTests::testQoS1PublishToClientIsDeliveredOnSessionResume()
{
    MqttClient *oldClient1 = connectAndWait("client1", true);
    QSignalSpy subscribedSpy(oldClient1, &MqttClient::subscribeResult);
    oldClient1->subscribe("/testtopic", Mqtt::QoS1);
    QTRY_VERIFY(subscribedSpy.count() == 1);

    // prevent the client from receiving anything
    oldClient1->d_ptr->transport->blockSignals(true);

    // publish something with a second client
    MqttClient *client2 = connectAndWait("client2");
    QSignalSpy publishedSpy(client2, &MqttClient::published);
    client2->publish("/testtopic", "Hello world", Mqtt::QoS1);
    QTRY_VERIFY(publishedSpy.count() == 1);

    // Resume (take over) old session and make sure we got the publish
    MqttClient *newClient1 = connectToServer("client1", false).first;
    QSignalSpy publishReceivedSpy(newClient1, &MqttClient::publishReceived);

    QTRY_VERIFY2(publishReceivedSpy.count() == 1, "Client did not receive publish packet upon session resume");
}

void MqttTests::testQoS2PublishToServerIsCompletedOnSessionResume()
{
    MqttClient *client = connectAndWait("client1", true);
    client->setAutoReconnect(true);

    QSignalSpy reconnectedSpy(client, &MqttClient::connected);

    QSignalSpy publishedSpy(client, &MqttClient::published);
    client->publish("/testtopic", "Hello world", Mqtt::QoS2);
    client->d_ptr->transport->flush();
    client->d_ptr->transport->abort();

    QVERIFY2(publishedSpy.count() == 0, "Should not have received the PUBACK yet... Test is bad.");

    QTRY_VERIFY2(reconnectedSpy.count() == 1, "client didn't reconnect");

    QTRY_VERIFY2(publishedSpy.count() == 1, "Published signal not emitted after reconnect");
}

void MqttTests::testQoS2PublishToClientIsCompletedOnSessionResume()
{
    MqttClient *oldClient1 = connectAndWait("client1", true);
    QSignalSpy subscribedSpy(oldClient1, &MqttClient::subscribeResult);
    oldClient1->subscribe("/testtopic", Mqtt::QoS2);
    QTRY_VERIFY(subscribedSpy.count() == 1);

    // prevent the client from receiving anything
    oldClient1->d_ptr->transport->blockSignals(true);

    // pbulish something with a second client
    MqttClient *client2 = connectAndWait("client2");
    QSignalSpy publishedSpy(client2, &MqttClient::published);
    client2->publish("/testtopic", "Hello world", Mqtt::QoS2);
    QTRY_VERIFY(publishedSpy.count() == 1);

    // Resume (take over) old session and make sure we got the publish
    MqttClient *newClient1 = connectToServer("client1", false).first;
    QSignalSpy publishReceivedSpy(newClient1, &MqttClient::publishReceived);

    QTRY_VERIFY2(publishReceivedSpy.count() == 1, "Client did not receive publish packet upon session resume");
}

void MqttTests::testRetain()
{
    MqttClient *client1 = connectAndWait("client1", true);

    // post a retained message
    QSignalSpy publishedSpy(client1, &MqttClient::published);
    client1->publish("/retaintopic", "Message 1", Mqtt::QoS1, true);
    QTRY_VERIFY(publishedSpy.count() == 1);

    // Connect a second client
    MqttClient *client2 = connectAndWait("client2");

    // subscribe to topic and verify we received the retained message
    QSignalSpy publishReceivedSpy(client2, &MqttClient::publishReceived);
    client2->subscribe("/retaintopic", Mqtt::QoS1);
    QTRY_VERIFY2(publishReceivedSpy.count() == 1, "Did not receive retained topic on subscribe.");
    QVERIFY2(publishReceivedSpy.first().at(2).toBool() == true, "Retain flag not set");

    publishReceivedSpy.clear();

    // Post another retained message from client1 and make sure we receive it
    client1->publish("/retaintopic", "Message 2", Mqtt::QoS1, true);
    QTRY_VERIFY2(publishReceivedSpy.count() == 1, "Did not receive published meessage.");
    QVERIFY2(publishReceivedSpy.first().at(2).toBool() == false, "Retain flag is set");

    // Disconnect client, and connect again, verify we get 2 retained messages now
    disconnectAndWait(client2);
    client2 = connectAndWait("client2");
    QSignalSpy publishReceivedSpy2(client2, &MqttClient::publishReceived);
    client2->subscribe("/retaintopic", Mqtt::QoS1);
    QTRY_VERIFY2(publishReceivedSpy2.count() == 2, "Did not receive retained topic on subscribe.");
    QVERIFY2(publishReceivedSpy2.at(0).at(2).toBool() == true, "Retain flag not set");
    QVERIFY2(publishReceivedSpy2.at(1).at(2).toBool() == true, "Retain flag not set");

    publishReceivedSpy2.clear();

    // Post a message with 0 paylod, it should be delivered as normal but discard any retained messages
    client1->publish("/retaintopic", QByteArray(), Mqtt::QoS1, true);
    QTRY_VERIFY2(publishReceivedSpy2.count() == 1, "Did not receive published message.");
    QVERIFY2(publishReceivedSpy.first().at(2).toBool() == false, "Retain flag is set");

    disconnectAndWait(client2);
    client2 = connectAndWait("client2");
    QSignalSpy publishReceivedSpy3(client2, &MqttClient::publishReceived);
    client2->subscribe("/retaintopic", Mqtt::QoS1);
    QTest::qWait(500);
    QVERIFY2(publishReceivedSpy3.count() == 0, "Did receive retained messages on subscribe but should not have.");

    // post another 2 retained messages (and some others), reconnect and verify they're there again
    client1->publish("/retaintopic", "Message 3", Mqtt::QoS1, true);
    client1->publish("/retaintopic", "Message 4", Mqtt::QoS1, false);
    client1->publish("/retaintopic", "Message 5", Mqtt::QoS1, false);
    client1->publish("/retaintopic", "Message 6", Mqtt::QoS1, true);
    client1->publish("/retaintopic", "Message 7", Mqtt::QoS1, false);
    QTRY_VERIFY(publishReceivedSpy3.count() == 5);

    disconnectAndWait(client2);
    client2 = connectAndWait("client2");
    QSignalSpy publishReceivedSpy4(client2, &MqttClient::publishReceived);
    client2->subscribe("/retaintopic", Mqtt::QoS1);
    QTRY_VERIFY2(publishReceivedSpy4.count() == 2, "Did not receive retained messages.");

    publishReceivedSpy4.clear();

    // post a QoS0 message to this topic. it should discard previously retained messages but stay retained
    client1->publish("/retaintopic", "Message 8", Mqtt::QoS0, true);
    QTRY_VERIFY2(publishReceivedSpy4.count() == 1, "Did not receive retained messages.");

    disconnectAndWait(client2);
    client2 = connectAndWait("client2");
    QSignalSpy publishReceivedSpy5(client2, &MqttClient::publishReceived);
    client2->subscribe("/retaintopic", Mqtt::QoS1);
    QTRY_VERIFY2(publishReceivedSpy5.count() == 1, "Did not receive exactly 1 retained message.");

    publishReceivedSpy5.clear();

    // post an empty payload to this topic. it should clear all retained messages
    client1->publish("/retaintopic", "", Mqtt::QoS1, true);
    QTRY_VERIFY2_WITH_TIMEOUT(publishReceivedSpy5.count() == 0, "Recaived a message but should not have.", 250);

    disconnectAndWait(client2);
    client2 = connectAndWait("client2");
    QSignalSpy publishReceivedSpy6(client2, &MqttClient::publishReceived);
    client2->subscribe("/retaintopic", Mqtt::QoS1);
    QTRY_VERIFY2_WITH_TIMEOUT(publishReceivedSpy6.count() == 0, "Recaived a message but should not have.", 250);

}

void MqttTests::testUnsubscribe()
{
    MqttClient *client1 = connectAndWait("client1");
    QVERIFY(subscribeAndWait(client1, "testtopic"));

    QSignalSpy publishReceivedSpy(client1, &MqttClient::publishReceived);

    MqttClient *client2 = connectAndWait("client2");
    client2->publish("testtopic", "Hello world");

    QTRY_VERIFY2(publishReceivedSpy.count() == 1, "Did not receive publish message");

    QSignalSpy unsubscribedSpy(client1, &MqttClient::unsubscribed);
    QSignalSpy serverSideUnsubscribedSpy(m_server, &MqttServer::clientUnsubscribed);

    quint16 packetId = client1->unsubscribe("testtopic");

    QTRY_VERIFY2(serverSideUnsubscribedSpy.count() == 1, "Server side unsubscribed signal not received");
    QVERIFY2(serverSideUnsubscribedSpy.first().at(0).toString() == "client1", "ClientId not matching");
    QVERIFY2(serverSideUnsubscribedSpy.first().at(1).toString() == "testtopic", "topicFilter not matching");

    QTRY_VERIFY2(unsubscribedSpy.count() == 1, "Unsubscibed signal not emitted");
    QVERIFY2(unsubscribedSpy.first().at(0).toInt() == packetId, "packet id not matching");

    publishReceivedSpy.clear();

    client2->publish("testtopic", "Hello world 2");

    QTest::qWait(500);
    QVERIFY2(publishReceivedSpy.count() == 0, "Received publish packet even though we should not have");
}

void MqttTests::testEmptyClientId()
{
    MqttClient *client1 = connectAndWait("");
    QVERIFY2(client1->isConnected(), "Client did not connect");

    MqttClient *client2 = connectAndWait("");
    QVERIFY2(client2->isConnected(), "Client did not connect");

    QPair<MqttClient*, QSignalSpy*> client3 = connectToServer("", false);
    QTRY_VERIFY2(client3.second->count() == 1, "Client did not emit connected signal");
    QTRY_COMPARE(client3.second->first().at(0).value<Mqtt::ConnectReturnCode>(), Mqtt::ConnectReturnCodeIdentifierRejected);
    QTRY_VERIFY2(client3.first->isConnected() == false, "Connection should have been dropped");
}

void MqttTests::testBinaryPaylaod()
{
    MqttClient *client = connectAndWait("");
    QVERIFY2(client->isConnected(), "Client did not connect");
    client->subscribe("#");
    const char binData[] = {'\xA5', '\x20', '\x00', '\x04', '\x00', '\x52'};
    QByteArray payload(QByteArray::fromRawData(binData, 6));
    QSignalSpy publishReceivedSpy(client, &MqttClient::publishReceived);
    client->publish("testtopic", payload);
    QTRY_VERIFY2(publishReceivedSpy.count() == 1, "Did not receive publish message");
    QCOMPARE(publishReceivedSpy.first().at(1).toByteArray(), payload);
}

#endif
