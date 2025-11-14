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

#ifndef AUTHORIZER_H
#define AUTHORIZER_H

#include "mqttpolicy.h"

#include <mqttserver.h>

#include <QObject>


class Authorizer : public QObject, public MqttAuthorizer
{
    Q_OBJECT
public:
    explicit Authorizer(const QString &policyFile, QObject *parent = nullptr);

    Mqtt::ConnectReturnCode authorizeConnect(int serverAddressId, const QString &clientId, const QString &username, const QString &password, const QHostAddress &peerAddress) override;
    bool authorizeSubscribe(int serverAddressId, const QString &clientId, const QString &topicFilter) override;
    bool authorizePublish(int serverAddressId, const QString &clientId, const QString &topic) override;

    void addPolicy(const QString &clientId, const QString &username, const QString &password, const QStringList &allowedSubscribeTopicFilters, const QStringList &allowedPublishTopicFilters);
    void removePolicy(const QString &clientId);

private:
    MqttPolicy loadPolicy(const QString &clientId);

private:
    QString m_settingsFile;

};

#endif // AUTHORIZER_H
