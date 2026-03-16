#include "contactdialog.h"
#include "ui_contactdialog.h"
#include "validator.h"

#include <QPushButton>

// The QDateEdit sentinel value used to represent "not set".
static const QDate kNullDate(1900, 1, 1);

ContactDialog::ContactDialog(QWidget *parent, const Contact &contact)
    : QDialog(parent)
    , ui(new Ui::ContactDialog)
    , m_contact(contact)
{
    ui->setupUi(this);

    // Style the error label in red.
    ui->labelError->setStyleSheet(
        QStringLiteral("QLabel { color: #c0392b; font-size: 11px; }"));
    ui->labelError->hide();

    // Configure the date edit sentinel (minimum = "not set").
    ui->dateEditBirthday->setMinimumDate(kNullDate);
    ui->dateEditBirthday->setMaximumDate(QDate::currentDate().addDays(-1));

    // Populate fields when editing an existing contact.
    if (contact.isPersisted()) {
        setWindowTitle(tr("Edit Contact"));
        ui->lineEditName->setText(contact.name);
        ui->lineEditMobile->setText(contact.mobile);
        ui->lineEditEmail->setText(contact.email);
        if (contact.birthday.isValid())
            ui->dateEditBirthday->setDate(contact.birthday);
        else
            ui->dateEditBirthday->setDate(kNullDate);
    } else {
        setWindowTitle(tr("Add Contact"));
        ui->dateEditBirthday->setDate(kNullDate);
    }

    // Connect OK button through our validation slot.
    connect(ui->buttonBox, &QDialogButtonBox::accepted,
            this, &ContactDialog::onAccepted);
}

ContactDialog::~ContactDialog()
{
    delete ui;
}

Contact ContactDialog::contact() const
{
    return m_contact;
}

void ContactDialog::onAccepted()
{
    if (validate())
        accept();
}

bool ContactDialog::validate()
{
    ui->labelError->hide();

    // Collect raw input.
    const QString name     = ui->lineEditName->text().trimmed();
    const QString mobile   = ui->lineEditMobile->text().trimmed();
    const QString email    = ui->lineEditEmail->text().trimmed();
    const QDate   birthday = ui->dateEditBirthday->date();
    const bool    bdSet    = birthday != kNullDate;

    // Run validators in priority order – stop at first error.
    QString error;
    error = Validator::validateName(name);
    if (error.isEmpty()) error = Validator::validateMobile(mobile);
    if (error.isEmpty()) error = Validator::validateEmail(email);
    if (error.isEmpty() && bdSet)
        error = Validator::validateBirthday(birthday.toString(Qt::ISODate));

    if (!error.isEmpty()) {
        ui->labelError->setText(error);
        ui->labelError->show();
        return false;
    }

    // Populate the output contact.
    m_contact.name     = name;
    m_contact.mobile   = mobile;
    m_contact.email    = email;
    m_contact.birthday = bdSet ? birthday : QDate{};
    return true;
}
