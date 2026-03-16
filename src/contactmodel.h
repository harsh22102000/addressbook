#pragma once

#include <QAbstractTableModel>
#include <QVector>
#include "contact.h"
#include "databasemanager.h"

/**
 * @brief Qt table model that exposes the contact list to QTableView.
 *
 * The model owns its in-memory cache and delegates all persistence to
 * DatabaseManager.  Views connect to the standard Qt model signals;
 * no direct coupling to the database is needed from the UI layer.
 */
class ContactModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum Column {
        ColName = 0,
        ColMobile,
        ColEmail,
        ColBirthday,
        ColCount        ///< Sentinel – do not use as a column index.
    };

    explicit ContactModel(DatabaseManager *db, QObject *parent = nullptr);

    // ---- QAbstractTableModel interface -------------------------------------
    int rowCount(const QModelIndex &parent = {}) const override;
    int columnCount(const QModelIndex &parent = {}) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // ---- Mutations ---------------------------------------------------------

    /// Add a new contact.  Returns true on success.
    bool addContact(Contact contact);

    /// Update the contact at the given row.  Returns true on success.
    bool updateContact(int row, const Contact &contact);

    /// Delete the contact at the given row.  Returns true on success.
    bool deleteContact(int row);

    /// Return a copy of the contact at row (read-only).
    Contact contactAt(int row) const;

    /// Reload all contacts from the database.
    void refresh();

    /// Forward the last database error.
    QString lastError() const;

private:
    DatabaseManager   *m_db;
    QVector<Contact>   m_contacts;
};
