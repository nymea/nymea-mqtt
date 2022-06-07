/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  Copyright 2013 - 2022, nymea GmbH
*  Contact: contact@nymea.io
*
*  This file is part of nymea.
*  This project including source code and documentation is protected by copyright law, and
*  remains the property of nymea GmbH. All rights, including reproduction, publication,
*  editing and translation, are reserved. The use of this project is subject to the terms of a
*  license agreement to be concluded with nymea GmbH in accordance with the terms
*  of use of nymea GmbH, available under https://nymea.io/license
*
*  GNU General Public License Usage
*  Alternatively, this project may be redistributed and/or modified under
*  the terms of the GNU General Public License as published by the Free Software Foundation,
*  GNU version 3. this project is distributed in the hope that it will be useful, but WITHOUT ANY
*  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
*  PURPOSE. See the GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License along with this project.
*  If not, see <https://www.gnu.org/licenses/>.
*
*  For any further details and any questions please contact us under contact@nymea.io
*  or see our FAQ/Licensing Information on https://nymea.io/license/faq
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "mqttpolicy.h"

MqttPolicy::MqttPolicy()
{

}

MqttPolicy::MqttPolicy(const QString &clientId, const QString &username, const QString &password, const QStringList &allowedSubscribeTopicFilters, const QStringList &allowedPublishTopicFilters):
    m_clientId(clientId),
    m_username(username),
    m_password(password),
    m_allowedSubscribeTopicFilters(allowedSubscribeTopicFilters),
    m_allowedPublishTopicFilters(allowedPublishTopicFilters)
{

}

QString MqttPolicy::clientId() const
{
    return m_clientId;
}

QString MqttPolicy::username() const
{
    return m_username;
}

QString MqttPolicy::password() const
{
    return m_password;
}

QStringList MqttPolicy::allowedSubscribeTopicFilters() const
{
    return m_allowedSubscribeTopicFilters;
}

QStringList MqttPolicy::allowedPublishTopicFilters() const
{
    return m_allowedPublishTopicFilters;
}

bool MqttPolicy::isValid() const
{
    return !m_clientId.isEmpty();
}
