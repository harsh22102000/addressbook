#include <QtTest/QtTest>
#include <QTemporaryFile>
#include "databasemanager.h"

/**
 * @brief Integration tests for DatabaseManager.
 *
 * Each test uses a fresh in-memory (temporary-file) SQLite database so
 * tests are fully isolated and leave no state on disk.
 */
class TestDatabaseManager : public QObject
{
    Q_OBJECT

private slots:
    void init();        ///< Called before each test.
    void cleanup();     ///< Called after each test.

    void openValidPath_isOpen();
    void openInvalidPath_notOpen();

    void addContact_assignsId();
    void addContact_persistsAllFields();

    void allContacts_emptyOnFreshDb();
    void allContacts_returnsInserted();
    void allContacts_sortedByName();

    void updateContact_changesFields();
    void updateContact_unknownId_noEffect();

    void deleteContact_removesRecord();
    void deleteContact_unknownId_noEffect();

    void contactById_found();
    void contactById_notFound_returnsDefault();

    void migrations_idempotent();

private:
    DatabaseManager *m_db = nullptr;
    QTemporaryFile   m_tmpFile;

    Contact makeContact(const QString &name,
                        const QString &mobile = {},
                        const QString &email  = {},
                        const QDate   &bday   = {}) const;
};

// ---------------------------------------------------------------------------
void TestDatabaseManager::init()
{
    m_tmpFile.setAutoRemove(true);
    QVERIFY(m_tmpFile.open());
    m_tmpFile.close();
    m_db = new DatabaseManager(m_tmpFile.fileName());
    QVERIFY(m_db->isOpen());
}

void TestDatabaseManager::cleanup()
{
    delete m_db;
    m_db = nullptr;
    // QSqlDatabase connections are per-name; remove to allow reuse.
    QSqlDatabase::removeDatabase(QStringLiteral("addressbook_conn"));
}

Contact TestDatabaseManager::makeContact(const QString &name,
                                         const QString &mobile,
                                         const QString &email,
                                         const QDate   &bday) const
{
    Contact c;
    c.name     = name;
    c.mobile   = mobile;
    c.email    = email;
    c.birthday = bday;
    return c;
}

// ---------------------------------------------------------------------------
void TestDatabaseManager::openValidPath_isOpen()
{
    QVERIFY(m_db->isOpen());
    QVERIFY(m_db->lastError().isEmpty());
}

void TestDatabaseManager::openInvalidPath_notOpen()
{
    QSqlDatabase::removeDatabase(QStringLiteral("addressbook_conn"));
    DatabaseManager bad(QStringLiteral("/nonexistent/path/ab.db"));
    QVERIFY(!bad.isOpen());
    QVERIFY(!bad.lastError().isEmpty());
    QSqlDatabase::removeDatabase(QStringLiteral("addressbook_conn"));
}

void TestDatabaseManager::addContact_assignsId()
{
    Contact c = makeContact(QStringLiteral("Alice"));
    QVERIFY(m_db->addContact(c));
    QVERIFY(c.id >= 0);
}

void TestDatabaseManager::addContact_persistsAllFields()
{
    Contact c = makeContact(QStringLiteral("Bob"),
                            QStringLiteral("+91 9876543210"),
                            QStringLiteral("bob@example.com"),
                            QDate(1985, 3, 22));
    QVERIFY(m_db->addContact(c));

    const Contact loaded = m_db->contactById(c.id);
    QCOMPARE(loaded.name,    c.name);
    QCOMPARE(loaded.mobile,  c.mobile);
    QCOMPARE(loaded.email,   c.email);
    QCOMPARE(loaded.birthday, c.birthday);
}

void TestDatabaseManager::allContacts_emptyOnFreshDb()
{
    QCOMPARE(m_db->allContacts().size(), 0);
}

void TestDatabaseManager::allContacts_returnsInserted()
{
    Contact a = makeContact(QStringLiteral("Alice"));
    Contact b = makeContact(QStringLiteral("Bob"));
    m_db->addContact(a);
    m_db->addContact(b);

    const auto all = m_db->allContacts();
    QCOMPARE(all.size(), 2);
}

void TestDatabaseManager::allContacts_sortedByName()
{
    for (const auto &n : {QStringLiteral("Zara"), QStringLiteral("Alice"),
                          QStringLiteral("Mike")}) {
        Contact c = makeContact(n);
        m_db->addContact(c);
    }
    const auto all = m_db->allContacts();
    QCOMPARE(all[0].name, QStringLiteral("Alice"));
    QCOMPARE(all[1].name, QStringLiteral("Mike"));
    QCOMPARE(all[2].name, QStringLiteral("Zara"));
}

void TestDatabaseManager::updateContact_changesFields()
{
    Contact c = makeContact(QStringLiteral("Charlie"));
    m_db->addContact(c);

    c.name   = QStringLiteral("Charles");
    c.mobile = QStringLiteral("1234567890");
    QVERIFY(m_db->updateContact(c));

    const Contact loaded = m_db->contactById(c.id);
    QCOMPARE(loaded.name,   QStringLiteral("Charles"));
    QCOMPARE(loaded.mobile, QStringLiteral("1234567890"));
}

void TestDatabaseManager::updateContact_unknownId_noEffect()
{
    Contact c = makeContact(QStringLiteral("Ghost"));
    c.id = 9999;
    // Should not crash, and the DB should still have 0 rows.
    m_db->updateContact(c);
    QCOMPARE(m_db->allContacts().size(), 0);
}

void TestDatabaseManager::deleteContact_removesRecord()
{
    Contact c = makeContact(QStringLiteral("Dave"));
    m_db->addContact(c);
    QVERIFY(m_db->deleteContact(c.id));
    QCOMPARE(m_db->allContacts().size(), 0);
}

void TestDatabaseManager::deleteContact_unknownId_noEffect()
{
    Contact c = makeContact(QStringLiteral("Eve"));
    m_db->addContact(c);
    m_db->deleteContact(9999); // should not crash
    QCOMPARE(m_db->allContacts().size(), 1);
}

void TestDatabaseManager::contactById_found()
{
    Contact c = makeContact(QStringLiteral("Frank"));
    m_db->addContact(c);
    const Contact loaded = m_db->contactById(c.id);
    QCOMPARE(loaded.id,   c.id);
    QCOMPARE(loaded.name, c.name);
}

void TestDatabaseManager::contactById_notFound_returnsDefault()
{
    const Contact c = m_db->contactById(9999);
    QCOMPARE(c.id, -1);
}

void TestDatabaseManager::migrations_idempotent()
{
    // Opening the same DB a second time should not fail (migrations already applied).
    QSqlDatabase::removeDatabase(QStringLiteral("addressbook_conn"));
    DatabaseManager db2(m_tmpFile.fileName());
    QVERIFY(db2.isOpen());
    QSqlDatabase::removeDatabase(QStringLiteral("addressbook_conn"));
}

QTEST_MAIN(TestDatabaseManager)
#include "tst_databasemanager.moc"
