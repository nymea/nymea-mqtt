#include "mqttserver.h"
#include "mqttclient.h"
#include "mqttclient_p.h"

#include <QTest>
#include <QSignalSpy>


class OperationTests: public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanup();

    void connectAndDisconnect();
    void keepAliveTimesOut();

    void subscribeAndPublish_data();
    void subscribeAndPublish();

    void willIsSentOnClientDisappearing();
    void willIsNotSentOnClientDisconnecting();

    void testWillRetain();

    void testAutoReconnect();

    void testQoS1Retransmissions();

    void testMultiSubscription();

    void testSubscriptionTopicFilters_data();
    void testSubscriptionTopicFilters();

    void testSubscriptionTopicMatching_data();
    void testSubscriptionTopicMatching();

    void testSessionManagementDropOldSession();
    void testSessionManagementResumeOldSession();
    void testSessionManagementFailResumeOldSession();

    void testQoS1PublishToServerIsAckedOnSessionResume();
    void testQoS1PublishToClientIsDeliveredOnSessionResume();

    void testQoS2PublishToServerIsCompletedOnSessionResume();

    void testQoS2PublishToClientIsCompletedOnSessionResume();

    void testRetain();

    void testUnsubscribe();

    void testEmptyClientId();

private:
    // Connects and waits for the MQTT CONNECT to be finished
    MqttClient *connectAndWait(const QString &clientId, bool cleanSession = true, quint16 keepAlive = 300, const QString &willTopic = QString(), const QString &willMessage = QString(), Mqtt::QoS willQoS = Mqtt::QoS0, bool willRetain = false);

    // Just connects, returns the client and signalspy which has been created before calling connect. You must delete the spy yourself!
    QPair<MqttClient*, QSignalSpy*> connectToServer(const QString &clientId, bool cleanSession = true, quint16 keepAlive = 300, const QString &willTopic = QString(), const QString &willMessage = QString(), Mqtt::QoS willQoS = Mqtt::QoS0, bool willRetain = false);

    void disconnectAndWait(MqttClient* client);

    bool subscribeAndWait(MqttClient* client, const QString &topic, Mqtt::QoS qos = Mqtt::QoS1);

private:
    QString m_serverHost = "127.0.0.1";
    quint16 m_serverPort = 5555;
    MqttServer *m_server = nullptr;

    QList<MqttClient*> m_clients;
};

MqttClient *OperationTests::connectAndWait(const QString &clientId, bool cleanSession, quint16 keepAlive, const QString &willTopic, const QString &willMessage, Mqtt::QoS willQoS, bool willRetain)
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

QPair<MqttClient*, QSignalSpy*> OperationTests::connectToServer(const QString &clientId, bool cleanSession, quint16 keepAlive, const QString &willTopic, const QString &willMessage, Mqtt::QoS willQoS, bool willRetain)
{
    MqttClient* client = new MqttClient(clientId, keepAlive, willTopic, willMessage.toUtf8(), willQoS, willRetain, this);
    client->setAutoReconnect(false);

    m_clients.append(client);

    QSignalSpy *spy = new QSignalSpy(client, &MqttClient::connected);
    client->connectToHost(m_serverHost, m_serverPort, cleanSession);
    return qMakePair<MqttClient*, QSignalSpy*>(client, spy);
}

void OperationTests::disconnectAndWait(MqttClient* client)
{
    QSignalSpy disconnectedSpy(client, &MqttClient::disconnected);
    client->disconnectFromHost();
    if (disconnectedSpy.count() == 0) {
        disconnectedSpy.wait();
    }
}

bool OperationTests::subscribeAndWait(MqttClient* client, const QString &topic, Mqtt::QoS qos)
{
    QSignalSpy subscribedSpy(client, &MqttClient::subscribed);
    quint16 packetId = client->subscribe(topic, qos);
    if (subscribedSpy.count() == 0) {
        subscribedSpy.wait();
    }
    Mqtt::SubscribeReturnCode expectedSubscribeReturnCode = qos == Mqtt::QoS0 ? Mqtt::SubscribeReturnCodeSuccessQoS0 : qos == Mqtt::QoS1 ? Mqtt::SubscribeReturnCodeSuccessQoS1 : Mqtt::SubscribeReturnCodeSuccessQoS2;
    return subscribedSpy.count() == 1 && subscribedSpy.first().at(0).toInt() == packetId && subscribedSpy.first().at(1).value<Mqtt::SubscribeReturnCodes>().first() == expectedSubscribeReturnCode;
}

void OperationTests::initTestCase()
{
//    QLoggingCategory::setFilterRules("nymea.mqtt.protocol.debug=false");

    m_server = new MqttServer(this);

    bool registered = false;
    quint16 attempts = 0;
    do {
        registered = m_server->listen(QHostAddress(m_serverHost), m_serverPort + attempts);
    } while(!registered && attempts++ < 20);

    QVERIFY2(registered, QString("Failed to register server on %1 from port %2 to %3. Tests won't work.").arg(m_serverHost).arg(m_serverPort).arg(m_serverPort+attempts).toUtf8().data());

    m_serverPort += attempts;
}

void OperationTests::cleanup()
{
    while (!m_clients.isEmpty()) {
        MqttClient *client = m_clients.takeFirst();
        client->disconnectFromHost();
        client->deleteLater();
    }
    QTRY_COMPARE(m_server->clients().count(), 0);
}

void OperationTests::connectAndDisconnect()
{
    QSignalSpy serverSpy(m_server, &MqttServer::clientConnected);

    QString clientId = "connectAndDisconnect-client";
    MqttClient* client = connectAndWait(clientId);

    QVERIFY2(serverSpy.count() == 1, "Server didn't emit clientConnected");
    QVERIFY2(serverSpy.at(0).first() == clientId, "ClientId not matching on server side.");

    QSignalSpy serverSpyDisconnect(m_server, &MqttServer::clientDisconnected);
    QSignalSpy clientSpy(client, &MqttClient::disconnected);
    client->disconnectFromHost();
    QTRY_VERIFY2(clientSpy.count() == 1, "client didn't emit disconnected");
    QTRY_VERIFY2(serverSpyDisconnect.count() == 1, "Server didn't emit clientDisconnected");
    QVERIFY2(serverSpyDisconnect.at(0).first() == clientId, "ClientId not matching on server side.");
}

void OperationTests::keepAliveTimesOut()
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

void OperationTests::subscribeAndPublish_data()
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

void OperationTests::subscribeAndPublish()
{
    QFETCH(Mqtt::QoS, qosClient1);
    QFETCH(Mqtt::QoS, qosClient2);

    QString clientId1 = QString("subQoS%1-client").arg(qosClient1);
    MqttClient *client1 = connectAndWait(clientId1);
    QString clientId2 = QString("pubQoS%1-client").arg(qosClient2);
    MqttClient *client2 = connectAndWait(clientId2);

    QSignalSpy serverSubscribeSpy(m_server, &MqttServer::clientSubscribed);
    QSignalSpy clientSubscribeSpy(client1, &MqttClient::subscribed);

    quint16 packetId = client1->subscribe("#", qosClient1);

    QTRY_VERIFY2(serverSubscribeSpy.count() == 1, "Server did not emit clientSubscribed");
    QVERIFY2(serverSubscribeSpy.first().first() == clientId1, "Client Id not matching");
    QVERIFY2(serverSubscribeSpy.first().at(1) == "#", "Topic not matching");
    QVERIFY2(serverSubscribeSpy.first().at(2) == qosClient1, "QoS not matching");

    QTRY_VERIFY2(clientSubscribeSpy.count() == 1, "Client did not emit subscribed");
    QVERIFY2(clientSubscribeSpy.first().first() == packetId, "Packet ID not matching");
    QVERIFY2(clientSubscribeSpy.first().at(1).value<Mqtt::SubscribeReturnCodes>().count() == 1, "Subscribe return code count not matching");

    QSignalSpy serverPublishReceivedSpy(m_server, &MqttServer::publishReceived);
    QSignalSpy serverPublishedSpy(m_server, &MqttServer::published);
    QSignalSpy client1PublishReceivedSpy(client1, &MqttClient::publishReceived);
    QSignalSpy client2PublishedSpy(client2, &MqttClient::published);

    packetId = client2->publish("/testtopic/", "Hello world", qosClient2);

    QTRY_VERIFY2(serverPublishReceivedSpy.count() == 1, "Server did not emit publishReceived");
    QVERIFY2(serverPublishReceivedSpy.first().at(0) == clientId2, "Server did emit publishReceived signal but client ID is not matching");
    QVERIFY2(serverPublishReceivedSpy.first().at(1) == packetId, QString("Server did emit publishReceived signal but Packet ID is not matching:\nActual: %1\nExpected: %2").arg(serverPublishReceivedSpy.first().at(1).toInt()).arg(packetId).toUtf8().data());
    QVERIFY2(serverPublishReceivedSpy.first().at(2) == "/testtopic/", "Server did emit publishReceived signal but topic is not matching");
    QVERIFY2(serverPublishReceivedSpy.first().at(3) == "Hello world", "Server did emit publishReceived signal but payload is not matching");

    QTRY_VERIFY2(serverPublishedSpy.count() == 1, "Server did not emit published");
    QVERIFY2(serverPublishedSpy.first().at(0) == clientId1, "Server did emit published signal but client ID is not matching");

    QTRY_VERIFY2(client1PublishReceivedSpy.count() == 1, "Subscribing client did not emit publishReceived signal");
    QVERIFY2(client1PublishReceivedSpy.first().at(0) == "/testtopic/", "Subscribing client did emit publishReceived signal but topic is not matching");
    QVERIFY2(client1PublishReceivedSpy.first().at(1) == "Hello world", "Subscribing client did emit publishReceived signal but payload is not matching");

    QTRY_VERIFY2(client2PublishedSpy.count() == 1, "Publishing client did not emit published signal");
    QVERIFY2(client2PublishedSpy.first().first() == packetId, "Publishing client did emit published signal but packet ID not matching");

}

void OperationTests::willIsSentOnClientDisappearing()
{
    MqttClient *client1 = connectAndWait("subWill-client");
    MqttClient *client2 = connectAndWait("pubWill-client", true, 300, "/testtopic", "Bye bye");

    QSignalSpy publishSpy(client1, &MqttClient::publishReceived);

    QVERIFY(subscribeAndWait(client1, "#"));

    client2->d_ptr->socket->abort();

    QTRY_VERIFY2(publishSpy.count() == 1, "Will has not been sent");
    QVERIFY2(publishSpy.first().at(0) == "/testtopic", "Will topic not matching");
    QVERIFY2(publishSpy.first().at(1) == "Bye bye", "Will message not matching");
}

void OperationTests::willIsNotSentOnClientDisconnecting()
{
    MqttClient *client1 = connectAndWait("subWill-client");
    MqttClient *client2 = connectAndWait("pubWill-client", true, 300, "/testtopic", "Bye bye");

    QSignalSpy subscribeSpy(client1, &MqttClient::subscribed);
    QSignalSpy publishSpy(client1, &MqttClient::publishReceived);

    client1->subscribe("#");
    subscribeSpy.wait();

    client2->disconnectFromHost();

    publishSpy.wait(200);
    QVERIFY2(publishSpy.count() == 0, "Will has been sent but it should not have been");
}

void OperationTests::testWillRetain()
{
    MqttClient *client1 = connectAndWait("subWill-client");
    MqttClient *client2 = connectAndWait("pubWill-client", true, 300, "/testtopic", "Bye bye", Mqtt::QoS1, true);

    QSignalSpy subscribeSpy(client1, &MqttClient::subscribed);
    QSignalSpy publishSpy(client1, &MqttClient::publishReceived);

    client1->subscribe("#");
    subscribeSpy.wait();

    client2->setAutoReconnect(false);
    client2->d_ptr->socket->abort();

    QTRY_VERIFY2(publishSpy.count() == 1, "Will has not been sent");
    QVERIFY2(publishSpy.first().at(0) == "/testtopic", QString("Will topic not matching: %1").arg(publishSpy.first().at(0).toString()).toUtf8().data());
    QVERIFY2(publishSpy.first().at(1) == "Bye bye", "Will message not matching");
    QVERIFY2(publishSpy.first().at(2) == false, "Retain flag not matching");

    MqttClient *client3 = connectAndWait("subWill-client2");
    QSignalSpy retainedWillSpy(client3, &MqttClient::publishReceived);

    client3->subscribe("#");
    QTRY_VERIFY2(retainedWillSpy.count() == 1, "Retained Will has not been sent");
    QVERIFY2(retainedWillSpy.first().at(0) == "/testtopic", "Will topic not matching");
    QVERIFY2(retainedWillSpy.first().at(1) == "Bye bye", "Will message not matching");
    QVERIFY2(retainedWillSpy.first().at(2) == true, "Retain flag not matching");

    // Clear retain on /testtopic
    QSignalSpy clearRetainSpy(client3, &MqttClient::published);
    client3->publish("/testtopic", QByteArray(), Mqtt::QoS1, true);
    QTRY_VERIFY2(clearRetainSpy.count() == 1, "Clearing retain message did not succeed");
}

void OperationTests::testAutoReconnect()
{
    MqttClient *client1 = connectAndWait("client1");
    client1->setAutoReconnect(true);

    QSignalSpy disconnectedSpy(client1, &MqttClient::disconnected);
    QSignalSpy connectedSpy(client1, &MqttClient::connected);

    client1->d_ptr->socket->abort();

    QTRY_VERIFY2(disconnectedSpy.count() == 1, "client did not emit disconnected");
    QTRY_VERIFY2(connectedSpy.count() == 1, "client did not emit connected");
}

void OperationTests::testQoS1Retransmissions()
{
    QSignalSpy serverSpy(m_server, &MqttServer::publishReceived);

    MqttClient *client = connectAndWait("client1");
    client->setAutoReconnect(true);

    // publish a packet, flush the pipe and immediately drop the connection before we have a chance to receive the PUBACK
    int packetId = client->publish("/testtopic", "Hello world", Mqtt::QoS1);
    client->d_ptr->socket->flush();
    QSignalSpy connectedSpy(client, &MqttClient::connected);
    client->d_ptr->socket->abort();

    // Wait for it to reconnect, it should then republish the packet
    connectedSpy.wait();

    QTRY_VERIFY2(serverSpy.count() == 2, "Server didn't receive the publication twice but it should have");
    QCOMPARE(serverSpy.at(0).at(0).toString(), QString("client1"));
    QCOMPARE(serverSpy.at(0).at(1).toInt(), packetId);
    QCOMPARE(serverSpy.at(0).at(2).toString(), QString("/testtopic"));
    QCOMPARE(serverSpy.at(0).at(3).toString(), QString("Hello world"));
    QCOMPARE(serverSpy.at(0).at(4).toBool(), false);

    QCOMPARE(serverSpy.at(1).at(0).toString(), QString("client1"));
    QCOMPARE(serverSpy.at(1).at(1).toInt(), packetId);
    QCOMPARE(serverSpy.at(1).at(2).toString(), QString("/testtopic"));
    QCOMPARE(serverSpy.at(1).at(3).toString(), QString("Hello world"));
    QCOMPARE(serverSpy.at(1).at(4).toBool(), true);
}

void OperationTests::testMultiSubscription()
{
    MqttClient *client = connectAndWait("subscription-topics");
    QSignalSpy subscribedSpy(client, &MqttClient::subscribed);

    MqttSubscriptions subscriptions = { MqttSubscription("topic1"), MqttSubscription("topic2") , MqttSubscription("#invalid") };
    Mqtt::SubscribeReturnCodes subscriptionReturnCodes = { Mqtt::SubscribeReturnCodeSuccessQoS0, Mqtt::SubscribeReturnCodeSuccessQoS0, Mqtt::SubscribeReturnCodeFailure};

    client->subscribe(subscriptions);
    QTRY_VERIFY2(subscribedSpy.count() == 1, "Subscribed signal not received");

    Mqtt::SubscribeReturnCodes retCodes = subscribedSpy.first().at(1).value<Mqtt::SubscribeReturnCodes>();
    QCOMPARE(retCodes, subscriptionReturnCodes);
}

void OperationTests::testSubscriptionTopicFilters_data()
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

void OperationTests::testSubscriptionTopicFilters()
{
    QFETCH(QString, topicFilter);
    QFETCH(Mqtt::SubscribeReturnCode, subscriptionReturnCode);

    MqttClient *client = connectAndWait("subscription-topics");
    QSignalSpy subscribedSpy(client, &MqttClient::subscribed);
    client->subscribe(topicFilter);
    QTRY_VERIFY2(subscribedSpy.count() == 1, "Subscribed signal not received");

    Mqtt::SubscribeReturnCodes retCodes = subscribedSpy.first().at(1).value<Mqtt::SubscribeReturnCodes>();
    QCOMPARE(retCodes.first(), subscriptionReturnCode);
}

void OperationTests::testSubscriptionTopicMatching_data()
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

    rows.append({ "$SYS/", "$SYS/", "0" });
    rows.append({ "#", "$SYS/", "0" });
    rows.append({ "+/", "$SYS/", "0" });

    foreach (const QStringList &row, rows) {
        QTest::newRow(QString("%1, %2").arg(row.at(0), row.at(1)).toUtf8().data()) << row.at(0) << row.at(1) << row.at(2).toInt();
    }
}

void OperationTests::testSubscriptionTopicMatching()
{
    QFETCH(QString, topicFilter);
    QFETCH(QString, topic);
    QFETCH(int, receivedPublishMessageCount);

    MqttClient *publisher = connectAndWait("publisher");
    MqttClient *subscriber = connectAndWait("subscriber");

    QSignalSpy subscribedSpy(subscriber, &MqttClient::subscribed);
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

void OperationTests::testSessionManagementDropOldSession()
{
    MqttClient *client1Session1 = connectAndWait("client1");
    client1Session1->setAutoReconnect(false);

    QSignalSpy subscribeSpy(client1Session1, &MqttClient::subscribed);
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

void OperationTests::testSessionManagementResumeOldSession()
{
    MqttClient *client1Session1 = connectAndWait("client1");
    client1Session1->setAutoReconnect(false);

    QSignalSpy subscribeSpy(client1Session1, &MqttClient::subscribed);
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

void OperationTests::testSessionManagementFailResumeOldSession()
{
    // try to resume non existing session
    QPair<MqttClient*, QSignalSpy*> client = connectToServer("client1", false);
    if (client.second->count() == 0) {
        client.second->wait();
    }
    QVERIFY2(!client.second->first().at(0).value<Mqtt::ConnackFlags>().testFlag(Mqtt::ConnackFlagSessionPresent), "Session present flag is set while it should not be.");
}

void OperationTests::testQoS1PublishToServerIsAckedOnSessionResume()
{
    MqttClient *client = connectAndWait("client1", true);
    client->setAutoReconnect(true);

    QSignalSpy reconnectedSpy(client, &MqttClient::connected);

    QSignalSpy publishedSpy(client, &MqttClient::published);
    client->publish("/testtopic", "Hello world", Mqtt::QoS1);
    client->d_ptr->socket->flush();
    client->d_ptr->socket->abort();

    QVERIFY2(publishedSpy.count() == 0, "Should not have received the PUBACK yet... Test is bad.");

    QTRY_VERIFY2(reconnectedSpy.count() == 1, "client didn't reconnect");

    QTRY_VERIFY2(publishedSpy.count() == 1, "Published signal not emitted after reconnect");

}

void OperationTests::testQoS1PublishToClientIsDeliveredOnSessionResume()
{
    MqttClient *oldClient1 = connectAndWait("client1", true);
    QSignalSpy subscribedSpy(oldClient1, &MqttClient::subscribed);
    oldClient1->subscribe("/testtopic", Mqtt::QoS1);
    QTRY_VERIFY(subscribedSpy.count() == 1);

    // prevent the client from receiving anything
    oldClient1->d_ptr->socket->blockSignals(true);

    // pbulish something with a second client
    MqttClient *client2 = connectAndWait("client2");
    QSignalSpy publishedSpy(client2, &MqttClient::published);
    client2->publish("/testtopic", "Hello world", Mqtt::QoS1);
    QTRY_VERIFY(publishedSpy.count() == 1);

    // Resume (take over) old session and make sure we got the publish
    MqttClient *newClient1 = new MqttClient("client1", this);
    m_clients.append(newClient1); // let cleanupTestcase() clean it up
    QSignalSpy publishReceivedSpy(newClient1, &MqttClient::publishReceived);

    newClient1->connectToHost(m_serverHost, m_serverPort, false);

    QTRY_VERIFY2(publishReceivedSpy.count() == 1, "Client did not receive publish packet upon session resume");
}

void OperationTests::testQoS2PublishToServerIsCompletedOnSessionResume()
{
    MqttClient *client = connectAndWait("client1", true);
    client->setAutoReconnect(true);

    QSignalSpy reconnectedSpy(client, &MqttClient::connected);

    QSignalSpy publishedSpy(client, &MqttClient::published);
    client->publish("/testtopic", "Hello world", Mqtt::QoS2);
    client->d_ptr->socket->flush();
    client->d_ptr->socket->abort();

    QVERIFY2(publishedSpy.count() == 0, "Should not have received the PUBACK yet... Test is bad.");

    QTRY_VERIFY2(reconnectedSpy.count() == 1, "client didn't reconnect");

    QTRY_VERIFY2(publishedSpy.count() == 1, "Published signal not emitted after reconnect");
}

void OperationTests::testQoS2PublishToClientIsCompletedOnSessionResume()
{
    MqttClient *oldClient1 = connectAndWait("client1", true);
    QSignalSpy subscribedSpy(oldClient1, &MqttClient::subscribed);
    oldClient1->subscribe("/testtopic", Mqtt::QoS2);
    QTRY_VERIFY(subscribedSpy.count() == 1);

    // prevent the client from receiving anything
    oldClient1->d_ptr->socket->blockSignals(true);

    // pbulish something with a second client
    MqttClient *client2 = connectAndWait("client2");
    QSignalSpy publishedSpy(client2, &MqttClient::published);
    client2->publish("/testtopic", "Hello world", Mqtt::QoS2);
    QTRY_VERIFY(publishedSpy.count() == 1);

    // Resume (take over) old session and make sure we got the publish
    MqttClient *newClient1 = new MqttClient("client1", this);
    m_clients.append(newClient1); // let cleanupTestcase() clean it up
    QSignalSpy publishReceivedSpy(newClient1, &MqttClient::publishReceived);

    newClient1->connectToHost(m_serverHost, m_serverPort, false);

    QTRY_VERIFY2(publishReceivedSpy.count() == 1, "Client did not receive publish packet upon session resume");
}

void OperationTests::testRetain()
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
}

void OperationTests::testUnsubscribe()
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

void OperationTests::testEmptyClientId()
{
    MqttClient *client1 = connectAndWait("");
    QVERIFY2(client1->isConnected(), "Client did not connect");

    MqttClient *client2 = connectAndWait("");
    QVERIFY2(client2->isConnected(), "Client did not connect");

    QPair<MqttClient*, QSignalSpy*> client3 = connectToServer("", false);
    QTRY_VERIFY2(client3.second->count() == 1, "Client did not emit connected signal");
    QTRY_COMPARE(client3.second->first().at(0).value<Mqtt::ConnectReturnCode>(), Mqtt::ConnectReturnCodeIdentifierRejected);
}


QTEST_MAIN(OperationTests)
#include "test_operation.moc"
