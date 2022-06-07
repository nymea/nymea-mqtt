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

#include "mqttserver.h"
#include "authorizer.h"
#include "certificateloader.h"

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QStandardPaths>
#include <QSettings>
#include <iostream>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString defaultConfigFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/nymea/nymea-mqtt-server.conf";
    QString defaultPolicyFile = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/nymea/mqttpolicies.conf";
    quint16 defaultTcpPort = 1883;
    quint16 defaultWsPort = 0;
    bool useSslDefault = false;
    QString defaultCertKeyFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/certs/certificate.key";
    QString defaultCertFileName = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/certs/certificate.crt";

    QCommandLineParser parser;
    parser.addOptions({
          {{"config", "c"}, QString("The configuration file to use (default: %1").arg(defaultConfigFile), "file", defaultConfigFile},
          {{"policy-file", "p"}, QString("The policy configuration file to use (default: %1)").arg(defaultPolicyFile), "file", defaultPolicyFile},
          {{"insecure", "i"}, "Run in insecure mode (allow all connections, publishes and subscribes)"},
          {{"tcp-port", "t"}, QString("The port for the TCP server (default: %1, 0 to disable)").arg(defaultTcpPort), "port", QString::number(defaultTcpPort)},
          {{"ws-port", "w"}, "The port for the web socket server (default: disabled)", "port", QString::number(defaultWsPort)},
          {{"add-policy", "a"}, "Add a new client policy"},
          {{"remove-policy", "r"}, "Remove a client policy", "clientId"},
          {{"ssl", "S"}, "Enable SSL encryption (default: disabled)"},
          {{"certificate", "C"}, QString("The SSL certificate to use (default: %1)").arg(defaultCertFileName), "crt file", defaultCertFileName},
          {{"certificate-key", "K"}, QString("The SSL certificate key to use (default: %1)").arg(defaultCertKeyFileName), "key file", defaultCertKeyFileName},
      });
    parser.setApplicationDescription("nymea-mqtt-server is a standalone MQTT broker with support for TCP and web socket connections.\n\n"
                                     "Every command line argument which can be passed, can also be set into the configuration file by specifing the long name for it followed by = and the desired value."
                                     "For example:\n\ntcp-port=1883\nssl=true\n\n"
                                     "Note that any passed command line arguments will still override any values set in the configuration file.\n\n"
                                     "Enabling SSL requires an SSL ertificate which can be configured with the certificate and certificate-key options. If no certificate is found in the given locations, a new self-signed certificate will be generated.\n\n"
                                     "Invoking the application with \"add-policy\" or \"remove-policy\" will allow changing the policies at run time, no broker restart is required. However, existing clients won't be disconnected immediately when a policy is removed but subsequent connect, subscribe or publish operations will be blocked.");
    parser.addHelpOption();

    parser.process(a.arguments());

    qDebug() << "Using configuration file:" << parser.value("config");
    QSettings settings(parser.value("config"), QSettings::IniFormat);
    QString policyFile = parser.isSet("policy-file") ? parser.value("policy-file") : settings.value("policy-file", defaultPolicyFile).toString();
    bool insecure = parser.isSet("insecure") ? true : settings.value("insecure", false).toBool();
    quint16 tcpPort = parser.isSet("tcp-port") ? parser.value("tcp-port").toUInt() : settings.value("tcp-port", defaultTcpPort).toUInt();
    quint16 wsPort = parser.isSet("ws-port") ? parser.value("ws-port").toUInt() : settings.value("ws-port", defaultWsPort).toUInt();
    bool useSsl = parser.isSet("ssl") || settings.value("ssl", useSslDefault).toBool();
    QString certificateKeyFile = parser.isSet("certificate-key") ? parser.value("certificate-key") : settings.value("certificate-key", defaultCertKeyFileName).toString();
    QString certificateFile = parser.isSet("certificate") ? parser.value("certificate") : settings.value("certificate", defaultCertFileName).toString();

    if (parser.isSet("add-policy")) {
        Authorizer authorizer(policyFile);
        std::string line;
        std::cout << "Client ID: ";
        std::getline(std::cin, line);
        QString clientId = QString::fromStdString(line);
        std::cout << "Username: ";
        std::getline(std::cin, line);
        QString username = QString::fromStdString(line);
        std::cout << "Password: ";
        std::getline(std::cin, line);
        QString password = QString::fromStdString(line);
        std::cout << "Subscribe topic filters (comma separated): ";
        std::getline(std::cin, line);
        QStringList allowedSubscribeTopicFilters = QString::fromStdString(line).split(",");
        std::cout << "Publish topic filters (comma separated): ";
        std::getline(std::cin, line);
        QStringList allowedPublishTopicFilters = QString::fromStdString(line).split(",");
        authorizer.addPolicy(clientId, username, password, allowedSubscribeTopicFilters, allowedPublishTopicFilters);
        exit(EXIT_SUCCESS);
    }
    if (parser.isSet("remove-policy")) {
        Authorizer authorizer(policyFile);
        authorizer.removePolicy(parser.value("remove-policy"));
        exit(EXIT_SUCCESS);
    }

    MqttServer server;

    Authorizer *authorizer = nullptr;
    if (!insecure) {
        authorizer = new Authorizer(policyFile);
        server.setAuthorizer(authorizer);
    }

    QSslConfiguration sslConfiguration;
    if (useSsl) {
        CertificateLoader certLoader;
        bool loaded = certLoader.loadCertificate(certificateKeyFile, certificateFile);
        if (!loaded) {
            certLoader.generateCertificate(certificateKeyFile, certificateFile);
            loaded = certLoader.loadCertificate(certificateKeyFile, certificateFile);
        }
        if (!loaded) {
            qCritical() << "Certificate files not found and unable to generate a new one.";
            exit(EXIT_FAILURE);
        }
        sslConfiguration.setProtocol(QSsl::TlsV1_2OrLater);
        sslConfiguration.setPrivateKey(certLoader.certificateKey());
        sslConfiguration.setLocalCertificate(certLoader.certificate());
    }

    if (tcpPort != 0) {
        int serverId = server.listen(QHostAddress::AnyIPv4, tcpPort, sslConfiguration);
        if (serverId == -1) {
            exit(EXIT_FAILURE);
        }
    }

    if (wsPort != 0) {
        int serverId = server.listenWebSocket(QHostAddress::AnyIPv4, wsPort, sslConfiguration);
        if (serverId == -1) {
            exit(EXIT_FAILURE);
        }
    }

    return a.exec();
}
