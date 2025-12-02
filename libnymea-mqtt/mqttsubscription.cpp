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
       \class MqttSubscription
       \brief A helper class for managing MQTT subscription filters
       \inmodule nymea-mqtt
       \ingroup mqtt

       Bundles topic filter and QoS type into a single data type.
*/

#include "mqttsubscription.h"

MqttSubscription::MqttSubscription()
{

}

MqttSubscription::MqttSubscription(const QByteArray &topicFilter, Mqtt::QoS qoS):
    m_topicFilter(topicFilter),
    m_qoS(qoS)
{

}

QByteArray MqttSubscription::topicFilter() const
{
    return m_topicFilter;
}

void MqttSubscription::setTopicFilter(const QByteArray &topicFilter)
{
    m_topicFilter = topicFilter;
}

Mqtt::QoS MqttSubscription::qoS() const
{
    return m_qoS;
}

void MqttSubscription::setQoS(Mqtt::QoS qoS)
{
    m_qoS = qoS;
}

bool MqttSubscription::operator==(const MqttSubscription &other) const
{
    return m_qoS == other.qoS() && m_topicFilter == other.topicFilter();
}
