// SPDX-License-Identifier: GPL-3.0-or-later

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
* Copyright (C) 2013 - 2024, nymea GmbH
* Copyright (C) 2024 - 2025, chargebyte austria GmbH
*
* This file is part of nymea-mqtt.
*
* nymea-mqtt is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* nymea-mqtt is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with nymea-mqtt. If not, see <https://www.gnu.org/licenses/>.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "certificateloader.h"

#include <openssl/ssl.h>

#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <QSaveFile>
#include <QUuid>
#include <QRegularExpression>

// Disabling deprecation warnings for openssl 3.0
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

CertificateLoader::CertificateLoader()
{

}

bool CertificateLoader::loadCertificate(const QString &certificateKeyFileName, const QString &certificateFileName)
{
    QFile certificateKeyFile(certificateKeyFileName);
    if (!certificateKeyFile.exists()) {
        qWarning() << "Could not load certificate key file" << certificateKeyFile.fileName() << "because the file does not exist.";
        return false;
    }

    if (!certificateKeyFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << certificateKeyFile.fileName() << ":" << certificateKeyFile.errorString();
        return false;
    }

    m_certificateKey = QSslKey(&certificateKeyFile, QSsl::Rsa);
    if (m_certificateKey.isNull()) {
        qWarning() << "SSL certificate key" << certificateFileName << "is not valid.";
        return false;
    }

    qDebug() << "Loaded private certificate key" << certificateKeyFileName;
    certificateKeyFile.close();

    QFile certificateFile(certificateFileName);
    if (!certificateFile.exists()) {
        qWarning() << "Could not load certificate file" << certificateFile.fileName() << "because the file does not exist.";
        return false;
    }

    if (!certificateFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open" << certificateFile.fileName() << ":" << certificateFile.errorString();
        return false;
    }

    m_certificate = QSslCertificate(&certificateFile);
    if (m_certificate.isNull()) {
        qWarning() << "SSL certificate" << certificateFileName << "is not valid.";;
        return false;
    }

    qDebug() << "Loaded certificate file" << certificateFileName;
    certificateFile.close();

    return true;
}

bool CertificateLoader::generateCertificate(const QString &certificateKeyFileName, const QString &certificateFileName)
{
    EVP_PKEY * pkey = nullptr;
    BIGNUM          *bne = NULL;
    RSA * rsa = nullptr;
    X509 * x509 = nullptr;
    X509_NAME * name = nullptr;
    BIO * bp_public = nullptr, * bp_private = nullptr;
    const char * keyBuffer = nullptr;
    const char * certBuffer = nullptr;

    QFileInfo certFi(certificateFileName);
    QFileInfo keyFi(certificateKeyFileName);

    QDir dir;
    if (!dir.mkpath(certFi.absolutePath()) || !dir.mkpath(keyFi.absolutePath())) {
        qWarning() << "Error creating SSL certificate destination folder";
        return false;
    }

    QSaveFile certfile(certificateFileName);
    QSaveFile keyFile(certificateKeyFileName);
    if (!certfile.open(QFile::WriteOnly | QFile::Truncate | QFile::Unbuffered) || !keyFile.open(QFile::WriteOnly | QFile::Truncate | QFile::Unbuffered)) {
        qWarning() << "Error opening SSL certificate files";
        return false;
    }

    bne = BN_new();
    BN_set_word(bne, RSA_F4);
    q_check_ptr(bne);

    rsa = RSA_new();
    RSA_generate_key_ex(rsa, 2048, bne, nullptr);
    q_check_ptr(rsa);

    pkey = EVP_PKEY_new();
    q_check_ptr(pkey);

    EVP_PKEY_assign_RSA(pkey, rsa);
    x509 = X509_new();
    q_check_ptr(x509);
    // Randomize serial number in case a previous one is stuck in a browser (Chromium
    // completely rejects reused serial numbers and doesn't even allow to bypass it by an exception)
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    std::srand(QUuid::createUuid().toString().remove(QRegularExpression("[a-zA-Z{}-]")).left(5).toInt());
#else
    qsrand(QUuid::createUuid().toString().remove(QRegExp("[a-zA-Z{}-]")).left(5).toInt());
#endif
    ASN1_INTEGER_set(X509_get_serialNumber(x509), std::rand());
    X509_gmtime_adj(X509_get_notBefore(x509), 0); // not before current time
    X509_gmtime_adj(X509_get_notAfter(x509), 31536000L*10); // not after 10 years from this point
    X509_set_pubkey(x509, pkey);
    name = X509_get_subject_name(x509);
    q_check_ptr(name);
    X509_NAME_add_entry_by_txt(name, "E", MBSTRING_ASC, (unsigned char *)"nymea", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char *)"nymea.io", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "OU", MBSTRING_ASC, (unsigned char *)"home", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char *)"nymea GmbH", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC, (unsigned char *)"Vienna", -1, -1, 0);
    X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"AT", -1, -1, 0);
    X509_set_issuer_name(x509, name);
    X509_sign(x509, pkey, EVP_sha256());
    bp_private = BIO_new(BIO_s_mem());
    q_check_ptr(bp_private);
    if(PEM_write_bio_PrivateKey(bp_private, pkey, nullptr, nullptr, 0, nullptr, nullptr) != 1)
    {
        BN_free(bne);
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free_all(bp_private);
        qWarning() << "PEM_write_bio_PrivateKey";
        return false;
    }
    bp_public = BIO_new(BIO_s_mem());
    q_check_ptr(bp_public);
    if(PEM_write_bio_X509(bp_public, x509) != 1)
    {

        BN_free(bne);
        EVP_PKEY_free(pkey);
        X509_free(x509);
        BIO_free_all(bp_public);
        BIO_free_all(bp_private);
        qWarning() << "PEM_write_bio_X509";
        return false;
    }
    long pubSize = BIO_get_mem_data(bp_public, &certBuffer);
    q_check_ptr(certBuffer);

    long privSize = BIO_get_mem_data(bp_private, &keyBuffer);
    q_check_ptr(keyBuffer);

    if (certfile.write(certBuffer, pubSize) == pubSize && keyFile.write(keyBuffer, privSize) == privSize) {
        certfile.commit();
        keyFile.commit();
        qDebug() << "Generated new SSL certificate";
    } else {
        qWarning() << "Error writing SSL certificate files" << certificateKeyFileName << certificateFileName;
        certfile.cancelWriting();
        keyFile.cancelWriting();
        BN_free(bne);
        EVP_PKEY_free(pkey); // this will also free the rsa key
        X509_free(x509);
        BIO_free_all(bp_public);
        BIO_free_all(bp_private);
        return false;
    }

    BN_free(bne);
    EVP_PKEY_free(pkey); // this will also free the rsa key
    X509_free(x509);
    BIO_free_all(bp_public);
    BIO_free_all(bp_private);

    return true;
}

QSslKey CertificateLoader::certificateKey() const
{
    return m_certificateKey;
}

QSslCertificate CertificateLoader::certificate() const
{
    return m_certificate;
}

#pragma GCC diagnostic pop
