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

#include "mqttserver.h"
#include "mqttclient.h"

#include <QTest>
#include <QSignalSpy>

#include "../common/mqtttests.h"

class TcpTests: public MqttTests
{
    Q_OBJECT

private:
    int startServer(MqttServer *server) override;
    void connectClientToServer(MqttClient *client, bool cleanSession) override;

    QString m_serverHost = "127.0.0.1";
    quint16 m_serverPort = 5555;

};

int TcpTests::startServer(MqttServer *server)
{
    return server->listen(QHostAddress(m_serverHost), m_serverPort);
}

void TcpTests::connectClientToServer(MqttClient *client, bool cleanSession)
{
    qDebug() << "Connecting to TCP";
    client->connectToHost(m_serverHost, m_serverPort, cleanSession);
}

QTEST_MAIN(TcpTests)

#include "test_tcp.moc"
