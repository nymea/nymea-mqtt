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
