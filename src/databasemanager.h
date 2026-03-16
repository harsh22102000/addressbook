#pragma once

#include <QObject>
#include <QSqlDatabase>
#include <QVector>
#include "contact.h"

/**
 * @brief Manages all SQLite persistence for the address book.
 *
 * Responsibilities
 * ----------------
 * - Open / create the SQLite database file.
 * - Apply schema migrations so the on-disk schema is always up-to-date.
 * - Expose CRUD operations for Contact records.
 *
 * Schema migrations
 * -----------------
 * Each migration is a numbered SQL block stored in applyMigrations().
 * A `schema_version` user-pragma tracks which migrations have been applied,
 * so adding a new migration never breaks an existing installation.
 */
class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    explicit DatabaseManager(const QString &dbPath, QObject *parent = nullptr);
    ~DatabaseManager() override;

    /// Returns true when the database was opened and migrated successfully.
    bool isOpen() const;

    /// Returns the last error string (empty when no error).
    QString lastError() const;

    // ---- CRUD ---------------------------------------------------------------

    /// Insert a new contact and set contact.id on success.  Returns true on success.
    bool addContact(Contact &contact);

    /// Update all fields for an already-persisted contact. Returns true on success.
    bool updateContact(const Contact &contact);

    /// Delete the contact with the given id. Returns true on success.
    bool deleteContact(int id);

    /// Retrieve all contacts ordered by name.
    QVector<Contact> allContacts() const;

    /// Retrieve a single contact by primary key. Returns Contact with id == -1 on miss.
    Contact contactById(int id) const;

private:
    bool applyMigrations();

    QSqlDatabase m_db;
    QString      m_lastError;
};
