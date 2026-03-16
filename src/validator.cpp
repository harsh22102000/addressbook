#include "validator.h"

#include <QRegularExpression>
#include <QDate>

namespace Validator {

QString validateName(const QString &name)
{
    const QString trimmed = name.trimmed();
    if (trimmed.isEmpty())
        return QStringLiteral("Name cannot be empty.");
    if (trimmed.length() > 100)
        return QStringLiteral("Name must not exceed 100 characters.");
    return {};
}

QString validateMobile(const QString &mobile)
{
    if (mobile.trimmed().isEmpty())
        return {}; // optional field

    // Strip allowed formatting characters to count raw digits.
    static const QRegularExpression kAllowed(
        QStringLiteral(R"([^\d+\-() ])"));
    if (mobile.contains(kAllowed))
        return QStringLiteral(
            "Mobile may only contain digits, spaces, '+', '-', '(' and ')'.");

    // Count digits only.
    int digits = 0;
    for (const QChar ch : mobile)
        if (ch.isDigit()) ++digits;

    if (digits < 7)
        return QStringLiteral("Mobile number must have at least 7 digits.");
    if (digits > 15)
        return QStringLiteral("Mobile number must not exceed 15 digits (ITU-T E.164).");

    return {};
}

QString validateEmail(const QString &email)
{
    if (email.trimmed().isEmpty())
        return {}; // optional field

    // RFC-5322 simplified pattern: local@domain.tld
    static const QRegularExpression kEmail(
        QStringLiteral(
            R"(^[a-zA-Z0-9._%+\-]+@[a-zA-Z0-9.\-]+\.[a-zA-Z]{2,}$)"));

    if (!kEmail.match(email.trimmed()).hasMatch())
        return QStringLiteral("Please enter a valid e-mail address (e.g. user@example.com).");

    return {};
}

QString validateBirthday(const QString &isoDate)
{
    if (isoDate.trimmed().isEmpty())
        return {}; // optional field

    const QDate date = QDate::fromString(isoDate, Qt::ISODate);
    if (!date.isValid())
        return QStringLiteral("Birthday must be a valid date (YYYY-MM-DD).");

    if (date >= QDate::currentDate())
        return QStringLiteral("Birthday must be in the past.");

    if (date.year() < 1900)
        return QStringLiteral("Birthday year must be 1900 or later.");

    return {};
}

} // namespace Validator
