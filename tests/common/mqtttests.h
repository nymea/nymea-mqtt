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


class MqttTests: public QObject
{
    Q_OBJECT

#if (QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))

private slots:
    void initTestCase();
    void cleanup();
    void cleanupTestCase();
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

    void testBinaryPaylaod();
#endif

private:
    // Connects and waits for the MQTT CONNECT to be finished
    MqttClient *connectAndWait(const QString &clientId, bool cleanSession = true, quint16 keepAlive = 300, const QString &willTopic = QString(), const QString &willMessage = QString(), Mqtt::QoS willQoS = Mqtt::QoS0, bool willRetain = false);

    // Just connects, returns the client and signalspy which has been created before calling connect. You must delete the spy yourself!
    QPair<MqttClient*, QSignalSpy*> connectToServer(const QString &clientId, bool cleanSession = true, quint16 keepAlive = 300, const QString &willTopic = QString(), const QString &willMessage = QString(), Mqtt::QoS willQoS = Mqtt::QoS0, bool willRetain = false);

    void disconnectAndWait(MqttClient* client);

    bool subscribeAndWait(MqttClient* client, const QString &topic, Mqtt::QoS qos = Mqtt::QoS1);

    virtual int startServer(MqttServer *server) = 0;
    virtual void connectClientToServer(MqttClient *client, bool cleanSession) = 0;

private:
    MqttServer *m_server = nullptr;
    int m_serverId = -1;

    QList<MqttClient*> m_clients;
};
