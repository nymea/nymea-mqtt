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
