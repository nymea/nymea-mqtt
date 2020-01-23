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
