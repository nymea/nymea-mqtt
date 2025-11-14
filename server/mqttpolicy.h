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

#ifndef MQTTPOLICY_H
#define MQTTPOLICY_H

#include <QString>
#include <QStringList>

class MqttPolicy
{
public:
    MqttPolicy();
    MqttPolicy(const QString &clientId, const QString &username, const QString &password, const QStringList &allowedSubscribeTopicFilters, const QStringList &allowedPublishTopicFilters);

    QString clientId() const;
    QString username() const;
    QString password() const;
    QStringList allowedSubscribeTopicFilters() const;
    QStringList allowedPublishTopicFilters() const;

    bool isValid() const;

private:
    QString m_clientId;
    QString m_username;
    QString m_password;
    QStringList m_allowedSubscribeTopicFilters;
    QStringList m_allowedPublishTopicFilters;
};

#endif // MQTTPOLICY_H
