#include <QtTest/QtTest>
#include "validator.h"

/**
 * @brief Unit tests for the Validator namespace.
 *
 * Each test method follows the pattern:
 *   - valid inputs produce an empty (no-error) string
 *   - invalid inputs produce a non-empty error message
 */
class TestValidator : public QObject
{
    Q_OBJECT

private slots:
    // ---- Name --------------------------------------------------------------
    void validateName_empty_returnsError();
    void validateName_tooLong_returnsError();
    void validateName_valid_returnsEmpty();
    void validateName_whitespaceOnly_returnsError();

    // ---- Mobile ------------------------------------------------------------
    void validateMobile_empty_returnsEmpty();        // optional field
    void validateMobile_valid_returnsEmpty();
    void validateMobile_tooShort_returnsError();
    void validateMobile_illegalChars_returnsError();
    void validateMobile_tooManyDigits_returnsError();

    // ---- Email -------------------------------------------------------------
    void validateEmail_empty_returnsEmpty();         // optional field
    void validateEmail_valid_returnsEmpty();
    void validateEmail_missingAt_returnsError();
    void validateEmail_missingDomain_returnsError();
    void validateEmail_missingTld_returnsError();

    // ---- Birthday ----------------------------------------------------------
    void validateBirthday_empty_returnsEmpty();      // optional field
    void validateBirthday_futureDate_returnsError();
    void validateBirthday_today_returnsError();
    void validateBirthday_valid_returnsEmpty();
    void validateBirthday_tooOld_returnsError();
    void validateBirthday_invalidString_returnsError();
};

// ---- Name ------------------------------------------------------------------
void TestValidator::validateName_empty_returnsError()
{
    QVERIFY(!Validator::validateName(QString{}).isEmpty());
}

void TestValidator::validateName_whitespaceOnly_returnsError()
{
    QVERIFY(!Validator::validateName(QStringLiteral("   ")).isEmpty());
}

void TestValidator::validateName_tooLong_returnsError()
{
    QVERIFY(!Validator::validateName(QString(101, QChar('a'))).isEmpty());
}

void TestValidator::validateName_valid_returnsEmpty()
{
    QVERIFY(Validator::validateName(QStringLiteral("Alice Smith")).isEmpty());
    QVERIFY(Validator::validateName(QStringLiteral("A")).isEmpty());
    QVERIFY(Validator::validateName(QString(100, QChar('b'))).isEmpty());
}

// ---- Mobile ----------------------------------------------------------------
void TestValidator::validateMobile_empty_returnsEmpty()
{
    QVERIFY(Validator::validateMobile(QString{}).isEmpty());
    QVERIFY(Validator::validateMobile(QStringLiteral("   ")).isEmpty());
}

void TestValidator::validateMobile_valid_returnsEmpty()
{
    QVERIFY(Validator::validateMobile(QStringLiteral("+91 98765 43210")).isEmpty());
    QVERIFY(Validator::validateMobile(QStringLiteral("(+1) 800-555-0100")).isEmpty());
    QVERIFY(Validator::validateMobile(QStringLiteral("1234567")).isEmpty());
}

void TestValidator::validateMobile_tooShort_returnsError()
{
    QVERIFY(!Validator::validateMobile(QStringLiteral("123456")).isEmpty()); // 6 digits
}

void TestValidator::validateMobile_illegalChars_returnsError()
{
    QVERIFY(!Validator::validateMobile(QStringLiteral("123abc456")).isEmpty());
    QVERIFY(!Validator::validateMobile(QStringLiteral("12@34567")).isEmpty());
}

void TestValidator::validateMobile_tooManyDigits_returnsError()
{
    QVERIFY(!Validator::validateMobile(QStringLiteral("1234567890123456")).isEmpty()); // 16
}

// ---- Email -----------------------------------------------------------------
void TestValidator::validateEmail_empty_returnsEmpty()
{
    QVERIFY(Validator::validateEmail(QString{}).isEmpty());
}

void TestValidator::validateEmail_valid_returnsEmpty()
{
    QVERIFY(Validator::validateEmail(QStringLiteral("user@example.com")).isEmpty());
    QVERIFY(Validator::validateEmail(QStringLiteral("a.b+c@sub.domain.org")).isEmpty());
}

void TestValidator::validateEmail_missingAt_returnsError()
{
    QVERIFY(!Validator::validateEmail(QStringLiteral("userexample.com")).isEmpty());
}

void TestValidator::validateEmail_missingDomain_returnsError()
{
    QVERIFY(!Validator::validateEmail(QStringLiteral("user@")).isEmpty());
}

void TestValidator::validateEmail_missingTld_returnsError()
{
    QVERIFY(!Validator::validateEmail(QStringLiteral("user@example")).isEmpty());
}

// ---- Birthday --------------------------------------------------------------
void TestValidator::validateBirthday_empty_returnsEmpty()
{
    QVERIFY(Validator::validateBirthday(QString{}).isEmpty());
}

void TestValidator::validateBirthday_futureDate_returnsError()
{
    const QString future = QDate::currentDate().addDays(1).toString(Qt::ISODate);
    QVERIFY(!Validator::validateBirthday(future).isEmpty());
}

void TestValidator::validateBirthday_today_returnsError()
{
    const QString today = QDate::currentDate().toString(Qt::ISODate);
    QVERIFY(!Validator::validateBirthday(today).isEmpty());
}

void TestValidator::validateBirthday_valid_returnsEmpty()
{
    QVERIFY(Validator::validateBirthday(QStringLiteral("1990-06-15")).isEmpty());
    QVERIFY(Validator::validateBirthday(QStringLiteral("1900-01-01")).isEmpty());
}

void TestValidator::validateBirthday_tooOld_returnsError()
{
    QVERIFY(!Validator::validateBirthday(QStringLiteral("1899-12-31")).isEmpty());
}

void TestValidator::validateBirthday_invalidString_returnsError()
{
    QVERIFY(!Validator::validateBirthday(QStringLiteral("not-a-date")).isEmpty());
}

QTEST_MAIN(TestValidator)
#include "tst_validator.moc"
