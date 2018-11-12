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

#ifndef MQTTSUBSCRIPTION_H
#define MQTTSUBSCRIPTION_H

#include "mqtt.h"
#include <QString>
#include <QtDebug>

class MqttSubscription
{
public:
    MqttSubscription();
    MqttSubscription(const QByteArray &topicFilter, Mqtt::QoS qoS = Mqtt::QoS0);

    QByteArray topicFilter() const;
    void setTopicFilter(const QByteArray &topicFilter);

    Mqtt::QoS qoS() const;
    void setQoS(Mqtt::QoS qoS);

    bool operator==(const MqttSubscription &other) const;
private:
    QByteArray m_topicFilter;
    Mqtt::QoS m_qoS = Mqtt::QoS0;

};
Q_DECLARE_METATYPE(MqttSubscription)

typedef QVector<MqttSubscription> MqttSubscriptions;
Q_DECLARE_METATYPE(MqttSubscriptions)

inline QDebug operator<<(QDebug debug, const MqttSubscription &subscription) {
    debug.nospace().noquote() << "\"" << subscription.topicFilter() << "\" QoS: " << subscription.qoS();
    return debug;
}

#endif // MQTTSUBSCRIPTION_H
