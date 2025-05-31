#include "simple2fa.h"

#include <QDateTime>
#include <QCryptographicHash>
#include <QMessageAuthenticationCode>
#include <QDebug>
#include<QRegularExpression>

QString Simple2FA::generateCode(const QString& base32Secret) {
    if (!isValidSecret(base32Secret)) {
        return "000000";
    }

    // Get current time step (30-second intervals)
    qint64 timeStep = QDateTime::currentSecsSinceEpoch() / 30;

    // Decode the base32 secret
    QByteArray key = base32Decode(base32Secret);
    if (key.isEmpty()) {
        return "000000";
    }

    // Convert time step to 8-byte array (big-endian)
    QByteArray timeBytes(8, 0);
    for (int i = 7; i >= 0; i--) {
        timeBytes[i] = static_cast<char>(timeStep & 0xFF);
        timeStep >>= 8;
    }

    // Generate HMAC-SHA1
    QByteArray hash = hmacSha1(key, timeBytes);

    // Dynamic truncation
    quint32 code = dynamicTruncate(hash);

    // Return 6-digit code with leading zeros
    return QString("%1").arg(code % 1000000, 6, 10, QChar('0'));
}

bool Simple2FA::isValidSecret(const QString& secret) {
    if (secret.isEmpty()) {
        return false;
    }

    // Check if all characters are valid base32
    const QString validChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    for (const QChar& c : secret) {
        if (!validChars.contains(c.toUpper())) {
            return false;
        }
    }

    // Check reasonable length (typically 16-32 characters)
    return secret.length() >= 8 && secret.length() <= 64;
}

int Simple2FA::getTimeRemaining() {
    return 30 - (QDateTime::currentSecsSinceEpoch() % 30);
}

QByteArray Simple2FA::base32Decode(const QString& input) {
    const QString alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    QByteArray result;
    QString cleanInput = input.toUpper().remove(QRegularExpression("[^A-Z2-7]"));

    if (cleanInput.isEmpty()) {
        return result;
    }

    int buffer = 0;
    int bitsLeft = 0;

    for (const QChar& c : cleanInput) {
        int value = alphabet.indexOf(c);
        if (value < 0) {
            continue; // Skip invalid characters
        }

        buffer = (buffer << 5) | value;
        bitsLeft += 5;

        if (bitsLeft >= 8) {
            result.append(static_cast<char>((buffer >> (bitsLeft - 8)) & 0xFF));
            bitsLeft -= 8;
        }
    }

    return result;
}

QByteArray Simple2FA::hmacSha1(const QByteArray& key, const QByteArray& data) {
    return QMessageAuthenticationCode::hash(data, key, QCryptographicHash::Sha1);
}

quint32 Simple2FA::dynamicTruncate(const QByteArray& hash) {
    if (hash.length() < 20) {
        return 0;
    }

    // Get the offset from the last nibble
    int offset = hash.at(19) & 0x0F;

    // Extract 4 bytes starting at offset
    quint32 code = ((static_cast<quint8>(hash.at(offset)) & 0x7F) << 24) |
                   ((static_cast<quint8>(hash.at(offset + 1)) & 0xFF) << 16) |
                   ((static_cast<quint8>(hash.at(offset + 2)) & 0xFF) << 8) |
                   (static_cast<quint8>(hash.at(offset + 3)) & 0xFF);

    return code;
}

