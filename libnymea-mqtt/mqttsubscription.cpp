/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                         *
 *  Copyright (C) 2018 Michael Zanetti <michael.zanetti@guh.io>            *
 *                                                                         *
 *  This file is part of nymea.                                            *
 *                                                                         *
 *  nymea is free software: you can redistribute it and/or modify          *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation, version 2 of the License.                *
 *                                                                         *
 *  nymea is distributed in the hope that it will be useful,               *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with nymea. If not, see <http://www.gnu.org/licenses/>.          *
 *                                                                         *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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
