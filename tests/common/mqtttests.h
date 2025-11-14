// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-mqtt.
*
* nymea-mqtt is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-mqtt is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-mqtt. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
