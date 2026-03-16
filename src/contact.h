#pragma once

#include <QString>
#include <QDate>

/**
 * @brief Plain data structure representing a single address book contact.
 *
 * All fields are stored as Qt value types so the struct is trivially
 * copyable and requires no heap allocation.
 */
struct Contact {
    int     id       = -1;          ///< Primary key (-1 = not yet persisted)
    QString name;                   ///< Full name
    QString mobile;                 ///< Mobile number (digits, +, -, spaces)
    QString email;                  ///< E-mail address
    QDate   birthday;               ///< Date of birth (may be null/invalid)

    /// Returns true when the contact has been saved to the database.
    bool isPersisted() const { return id >= 0; }
};
