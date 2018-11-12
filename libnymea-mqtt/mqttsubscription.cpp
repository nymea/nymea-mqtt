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
