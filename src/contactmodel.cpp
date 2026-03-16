#include "contactmodel.h"
#include <QFont>
#include <QColor>

// ---------------------------------------------------------------------------
ContactModel::ContactModel(DatabaseManager *db, QObject *parent)
    : QAbstractTableModel(parent), m_db(db)
{
    refresh();
}

// ---------------------------------------------------------------------------
// QAbstractTableModel
// ---------------------------------------------------------------------------
int ContactModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return m_contacts.size();
}

int ContactModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    return ColCount;
}

QVariant ContactModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_contacts.size())
        return {};

    const Contact &c = m_contacts.at(index.row());

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
        case ColName:     return c.name;
        case ColMobile:   return c.mobile.isEmpty() ? QStringLiteral("—") : c.mobile;
        case ColEmail:    return c.email.isEmpty()  ? QStringLiteral("—") : c.email;
        case ColBirthday:
            return c.birthday.isValid()
                       ? c.birthday.toString(QStringLiteral("dd MMM yyyy"))
                       : QStringLiteral("—");
        }
    }

    if (role == Qt::ForegroundRole) {
        const bool empty =
            (index.column() == ColMobile  && c.mobile.isEmpty())  ||
            (index.column() == ColEmail   && c.email.isEmpty())   ||
            (index.column() == ColBirthday && !c.birthday.isValid());
        if (empty)
            return QColor(Qt::gray);
    }

    if (role == Qt::TextAlignmentRole)
        return Qt::AlignVCenter | Qt::AlignLeft;

    return {};
}

QVariant ContactModel::headerData(int section, Qt::Orientation orientation,
                                   int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return {};

    switch (section) {
    case ColName:     return QStringLiteral("Name");
    case ColMobile:   return QStringLiteral("Mobile");
    case ColEmail:    return QStringLiteral("Email");
    case ColBirthday: return QStringLiteral("Birthday");
    }
    return {};
}

// ---------------------------------------------------------------------------
// Mutations
// ---------------------------------------------------------------------------
bool ContactModel::addContact(Contact contact)
{
    if (!m_db->addContact(contact))
        return false;

    // Insert into the sorted position (by name, case-insensitive).
    const QString nameLower = contact.name.toLower();
    int insertPos = 0;
    while (insertPos < m_contacts.size() &&
           m_contacts.at(insertPos).name.toLower() <= nameLower)
        ++insertPos;

    beginInsertRows({}, insertPos, insertPos);
    m_contacts.insert(insertPos, contact);
    endInsertRows();
    return true;
}

bool ContactModel::updateContact(int row, const Contact &contact)
{
    if (row < 0 || row >= m_contacts.size()) return false;
    if (!m_db->updateContact(contact)) return false;

    m_contacts[row] = contact;
    emit dataChanged(index(row, 0), index(row, ColCount - 1));
    return true;
}

bool ContactModel::deleteContact(int row)
{
    if (row < 0 || row >= m_contacts.size()) return false;
    const int id = m_contacts.at(row).id;
    if (!m_db->deleteContact(id)) return false;

    beginRemoveRows({}, row, row);
    m_contacts.removeAt(row);
    endRemoveRows();
    return true;
}

Contact ContactModel::contactAt(int row) const
{
    if (row < 0 || row >= m_contacts.size()) return {};
    return m_contacts.at(row);
}

void ContactModel::refresh()
{
    beginResetModel();
    m_contacts = m_db->allContacts();
    endResetModel();
}

QString ContactModel::lastError() const
{
    return m_db->lastError();
}
