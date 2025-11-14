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

#include "authorizer.h"

#include <QFile>
#include <QSettings>

Authorizer::Authorizer(const QString &policyFile, QObject *parent):
    QObject{parent},
    m_settingsFile(policyFile)
{
    if (QFile::exists(policyFile)) {
        qInfo() << "Using policy file:" << policyFile;
    }

}

Mqtt::ConnectReturnCode Authorizer::authorizeConnect(int serverAddressId, const QString &clientId, const QString &username, const QString &password, const QHostAddress &peerAddress)
{
    Q_UNUSED(serverAddressId)
    Q_UNUSED(peerAddress);

    if (!QFile::exists(m_settingsFile)) {
        return Mqtt::ConnectReturnCodeServerUnavailable;
    }
    MqttPolicy policy = loadPolicy(clientId);
    if (!policy.isValid()) {
        return Mqtt::ConnectReturnCodeNotAuthorized;
    }
    if (policy.username() != username || policy.password() != password) {
        return Mqtt::ConnectReturnCodeBadUsernameOrPassword;
    }
    return Mqtt::ConnectReturnCodeAccepted;
}

bool Authorizer::authorizeSubscribe(int serverAddressId, const QString &clientId, const QString &topicFilter)
{
    Q_UNUSED(serverAddressId)

    qCritical() << "sub filters" << topicFilter;
    if (!QFile::exists(m_settingsFile)) {
        return false;
    }
    MqttPolicy policy = loadPolicy(clientId);
    if (!policy.isValid()) {
        return false;
    }
    qCritical() << "policy" << policy.allowedSubscribeTopicFilters();
    if (policy.allowedSubscribeTopicFilters().contains(topicFilter)) {
        return true;
    }
    return false;
}

bool Authorizer::authorizePublish(int serverAddressId, const QString &clientId, const QString &topic)
{
    Q_UNUSED(serverAddressId)

    if (!QFile::exists(m_settingsFile)) {
        return false;
    }
    MqttPolicy policy = loadPolicy(clientId);
    if (!policy.isValid()) {
        return false;
    }
    if (policy.allowedPublishTopicFilters().contains(topic)) {
        return true;
    }
    return false;
}

void Authorizer::addPolicy(const QString &clientId, const QString &username, const QString &password, const QStringList &allowedSubscribeTopicFilters, const QStringList &allowedPublishTopicFilters)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.beginGroup(clientId);
    settings.setValue("username", username);
    settings.setValue("password", password);
    settings.setValue("allowedSubscribeTopicFilters", allowedSubscribeTopicFilters);
    settings.setValue("allowedPublishTopicFilters", allowedPublishTopicFilters);
}

void Authorizer::removePolicy(const QString &clientId)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    settings.remove(clientId);
}

MqttPolicy Authorizer::loadPolicy(const QString &clientId)
{
    QSettings settings(m_settingsFile, QSettings::IniFormat);
    if (!settings.childGroups().contains(clientId)) {
        return MqttPolicy();
    }
    settings.beginGroup(clientId);
    MqttPolicy policy(clientId,
                      settings.value("username").toString(),
                      settings.value("password").toString(),
                      settings.value("allowedSubscribeTopicFilters").toStringList(),
                      settings.value("allowedPublishTopicFilters").toStringList());
    return policy;
}
