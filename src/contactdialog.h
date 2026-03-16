#pragma once

#include <QDialog>
#include "contact.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ContactDialog; }
QT_END_NAMESPACE

/**
 * @brief Modal dialog for creating or editing a contact.
 *
 * Pass an existing Contact to edit it; leave the default Contact (id == -1)
 * to create a new one.  Call contact() after exec() returns QDialog::Accepted
 * to retrieve the validated data.
 */
class ContactDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ContactDialog(QWidget *parent = nullptr,
                           const Contact &contact = {});
    ~ContactDialog() override;

    /// Returns the contact populated from the form fields.
    Contact contact() const;

private slots:
    void onAccepted();

private:
    /// Validate all fields and populate m_contact.  Returns true if valid.
    bool validate();

    Ui::ContactDialog *ui;
    Contact            m_contact;
};
