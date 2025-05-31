#ifndef SIMPLE2FA_H
#define SIMPLE2FA_H

#include <QString>
#include <QByteArray>

class Simple2FA
{
public:
    static QString generateCode(const QString& base32Secret);
    static bool isValidSecret(const QString& secret);
    static int getTimeRemaining(); // Seconds until next code (0-29)

private:
    static QByteArray base32Decode(const QString& input);
    static QByteArray hmacSha1(const QByteArray& key, const QByteArray& data);
    static quint32 dynamicTruncate(const QByteArray& hash);
};

#endif // SIMPLE2FA_H

