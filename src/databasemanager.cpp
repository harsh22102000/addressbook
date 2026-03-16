#include "databasemanager.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

// ---------------------------------------------------------------------------
// Migration definitions
// ---------------------------------------------------------------------------
// Each entry is {migration_id, sql}.  migration_id must be monotonically
// increasing integers starting at 1.
static const QVector<QPair<int, QString>> kMigrations = {
    {
        1,
        QStringLiteral(
            "CREATE TABLE IF NOT EXISTS contacts ("
            "  id       INTEGER PRIMARY KEY AUTOINCREMENT,"
            "  name     TEXT    NOT NULL,"
            "  mobile   TEXT    NOT NULL DEFAULT '',"
            "  email    TEXT    NOT NULL DEFAULT '',"
            "  birthday TEXT    NOT NULL DEFAULT ''"   // stored as ISO-8601 yyyy-MM-dd
            ");"
        )
    },
    // Future migrations go here, e.g.:
    // { 2, "ALTER TABLE contacts ADD COLUMN notes TEXT NOT NULL DEFAULT '';" },
};

// ---------------------------------------------------------------------------
DatabaseManager::DatabaseManager(const QString &dbPath, QObject *parent)
    : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"),
                                     QStringLiteral("addressbook_conn"));
    m_db.setDatabaseName(dbPath);

    if (!m_db.open()) {
        m_lastError = m_db.lastError().text();
        qCritical() << "DatabaseManager: cannot open DB:" << m_lastError;
        return;
    }

    // Enable WAL mode and foreign keys for better concurrency & integrity.
    QSqlQuery pragma(m_db);
    pragma.exec(QStringLiteral("PRAGMA journal_mode=WAL;"));
    pragma.exec(QStringLiteral("PRAGMA foreign_keys=ON;"));

    if (!applyMigrations()) {
        qCritical() << "DatabaseManager: migration failed:" << m_lastError;
        m_db.close();
    }
}

DatabaseManager::~DatabaseManager()
{
    if (m_db.isOpen())
        m_db.close();
}

bool DatabaseManager::isOpen() const
{
    return m_db.isOpen();
}

QString DatabaseManager::lastError() const
{
    return m_lastError;
}

// ---------------------------------------------------------------------------
// Schema migrations
// ---------------------------------------------------------------------------
bool DatabaseManager::applyMigrations()
{
    // Bootstrap: create a table that records applied migration ids.
    {
        QSqlQuery q(m_db);
        if (!q.exec(QStringLiteral(
                "CREATE TABLE IF NOT EXISTS schema_migrations ("
                "  version INTEGER PRIMARY KEY"
                ");"))) {
            m_lastError = q.lastError().text();
            return false;
        }
    }

    for (const auto &[version, sql] : kMigrations) {
        // Check whether this migration was already applied.
        QSqlQuery check(m_db);
        check.prepare(QStringLiteral(
            "SELECT COUNT(*) FROM schema_migrations WHERE version = :v;"));
        check.bindValue(QStringLiteral(":v"), version);
        if (!check.exec() || !check.next()) {
            m_lastError = check.lastError().text();
            return false;
        }
        if (check.value(0).toInt() > 0)
            continue; // already applied

        // Apply migration inside a transaction.
        m_db.transaction();
        QSqlQuery mq(m_db);
        if (!mq.exec(sql)) {
            m_lastError = mq.lastError().text();
            m_db.rollback();
            return false;
        }
        QSqlQuery mark(m_db);
        mark.prepare(QStringLiteral(
            "INSERT INTO schema_migrations (version) VALUES (:v);"));
        mark.bindValue(QStringLiteral(":v"), version);
        if (!mark.exec()) {
            m_lastError = mark.lastError().text();
            m_db.rollback();
            return false;
        }
        m_db.commit();
        qDebug() << "DatabaseManager: applied migration" << version;
    }
    return true;
}

// ---------------------------------------------------------------------------
// CRUD helpers
// ---------------------------------------------------------------------------
bool DatabaseManager::addContact(Contact &contact)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "INSERT INTO contacts (name, mobile, email, birthday)"
        " VALUES (:name, :mobile, :email, :birthday);"));
    q.bindValue(QStringLiteral(":name"),     contact.name);
    q.bindValue(QStringLiteral(":mobile"),   contact.mobile);
    q.bindValue(QStringLiteral(":email"),    contact.email);
    q.bindValue(QStringLiteral(":birthday"),
                contact.birthday.isValid()
                    ? contact.birthday.toString(Qt::ISODate)
                    : QString{});

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    contact.id = q.lastInsertId().toInt();
    return true;
}

bool DatabaseManager::updateContact(const Contact &contact)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "UPDATE contacts"
        " SET name=:name, mobile=:mobile, email=:email, birthday=:birthday"
        " WHERE id=:id;"));
    q.bindValue(QStringLiteral(":name"),     contact.name);
    q.bindValue(QStringLiteral(":mobile"),   contact.mobile);
    q.bindValue(QStringLiteral(":email"),    contact.email);
    q.bindValue(QStringLiteral(":birthday"),
                contact.birthday.isValid()
                    ? contact.birthday.toString(Qt::ISODate)
                    : QString{});
    q.bindValue(QStringLiteral(":id"),       contact.id);

    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::deleteContact(int id)
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral("DELETE FROM contacts WHERE id=:id;"));
    q.bindValue(QStringLiteral(":id"), id);
    if (!q.exec()) {
        m_lastError = q.lastError().text();
        return false;
    }
    return true;
}

QVector<Contact> DatabaseManager::allContacts() const
{
    QVector<Contact> result;
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT id, name, mobile, email, birthday FROM contacts ORDER BY name COLLATE NOCASE;"));
    if (!q.exec()) {
        qWarning() << "DatabaseManager::allContacts error:" << q.lastError().text();
        return result;
    }
    while (q.next()) {
        Contact c;
        c.id       = q.value(0).toInt();
        c.name     = q.value(1).toString();
        c.mobile   = q.value(2).toString();
        c.email    = q.value(3).toString();
        const QString bd = q.value(4).toString();
        if (!bd.isEmpty())
            c.birthday = QDate::fromString(bd, Qt::ISODate);
        result.append(c);
    }
    return result;
}

Contact DatabaseManager::contactById(int id) const
{
    QSqlQuery q(m_db);
    q.prepare(QStringLiteral(
        "SELECT id, name, mobile, email, birthday FROM contacts WHERE id=:id;"));
    q.bindValue(QStringLiteral(":id"), id);
    if (q.exec() && q.next()) {
        Contact c;
        c.id       = q.value(0).toInt();
        c.name     = q.value(1).toString();
        c.mobile   = q.value(2).toString();
        c.email    = q.value(3).toString();
        const QString bd = q.value(4).toString();
        if (!bd.isEmpty())
            c.birthday = QDate::fromString(bd, Qt::ISODate);
        return c;
    }
    return Contact{}; // id == -1
}
