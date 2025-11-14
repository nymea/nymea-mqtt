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

#include "mqttpolicy.h"

MqttPolicy::MqttPolicy()
{

}

MqttPolicy::MqttPolicy(const QString &clientId, const QString &username, const QString &password, const QStringList &allowedSubscribeTopicFilters, const QStringList &allowedPublishTopicFilters):
    m_clientId(clientId),
    m_username(username),
    m_password(password),
    m_allowedSubscribeTopicFilters(allowedSubscribeTopicFilters),
    m_allowedPublishTopicFilters(allowedPublishTopicFilters)
{

}

QString MqttPolicy::clientId() const
{
    return m_clientId;
}

QString MqttPolicy::username() const
{
    return m_username;
}

QString MqttPolicy::password() const
{
    return m_password;
}

QStringList MqttPolicy::allowedSubscribeTopicFilters() const
{
    return m_allowedSubscribeTopicFilters;
}

QStringList MqttPolicy::allowedPublishTopicFilters() const
{
    return m_allowedPublishTopicFilters;
}

bool MqttPolicy::isValid() const
{
    return !m_clientId.isEmpty();
}
