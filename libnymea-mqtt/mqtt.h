#ifndef MQTT_H
#define MQTT_H

#include <QObject>

namespace Mqtt {

enum Protocol {
    ProtocolUnknown = 0x00,
    Protocol311 = 0x04
};

enum QoS {
    QoS0 = 0x00,
    QoS1 = 0x01,
    QoS2 = 0x02
};

enum ConnectFlag {
    ConnectFlagNone = 0x00,
    ConnectFlagCleanSession = 0x02,
    ConnectFlagWill = 0x04,
    ConnectFlagWillQoS1 = 0x08,
    ConnectFlagWillQoS2 = 0x10,
    ConnectFlagWillRetain = 0x20,
    ConnectFlagPassword = 0x40,
    ConnectFlagUsername = 0x80
};
Q_DECLARE_FLAGS(ConnectFlags, ConnectFlag)

enum ConnackFlag {
    ConnackFlagNone = 0x0,
    ConnackFlagSessionPresent = 0x1
};
Q_DECLARE_FLAGS(ConnackFlags, ConnackFlag)

enum ConnectReturnCode {
    ConnectReturnCodeAccepted = 0x00,
    ConnectReturnCodeUnacceptableProtocolVersion = 0x01,
    ConnectReturnCodeIdentifierRejected = 0x02,
    ConnectReturnCodeServerUnavailable = 0x03,
    ConnectReturnCodeBadUsernameOrPassword = 0x04,
    ConnectReturnCodeNotAuthorized = 0x05
};
enum SubscribeReturnCode {
    SubscribeReturnCodeSuccessQoS0 = 0x00,
    SubscribeReturnCodeSuccessQoS1 = 0x01,
    SubscribeReturnCodeSuccessQoS2 = 0x02,
    SubscribeReturnCodeFailure = 0x80
};
typedef QList<SubscribeReturnCode> SubscribeReturnCodes;

};

Q_DECLARE_METATYPE(Mqtt::QoS)
Q_DECLARE_METATYPE(Mqtt::ConnectFlags)
Q_DECLARE_METATYPE(Mqtt::ConnackFlags)
Q_DECLARE_METATYPE(Mqtt::ConnectReturnCode)
Q_DECLARE_METATYPE(Mqtt::SubscribeReturnCode)
Q_DECLARE_METATYPE(Mqtt::SubscribeReturnCodes)

#endif // MQTT_H
