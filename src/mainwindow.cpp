#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "contactdialog.h"

#include <QSortFilterProxyModel>
#include <QHeaderView>
#include <QMessageBox>
#include <QAction>
#include <QIcon>
#include <QStandardPaths>
#include <QDir>
#include <QKeySequence>

// ---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // ---- Database ----------------------------------------------------------
    const QString dataDir =
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    const QString dbPath = dataDir + QStringLiteral("/addressbook.db");

    m_db = new DatabaseManager(dbPath, this);
    if (!m_db->isOpen()) {
        QMessageBox::critical(this, tr("Database Error"),
                              tr("Could not open the database:\n%1")
                                  .arg(m_db->lastError()));
    }

    // ---- Model + Proxy -----------------------------------------------------
    m_model = new ContactModel(m_db, this);
    m_proxy = new QSortFilterProxyModel(this);
    m_proxy->setSourceModel(m_model);
    m_proxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxy->setFilterKeyColumn(-1); // search all columns

    // ---- Table view --------------------------------------------------------
    ui->tableView->setModel(m_proxy);

    QHeaderView *hh = ui->tableView->horizontalHeader();
    hh->setSectionResizeMode(ContactModel::ColName,     QHeaderView::Stretch);
    hh->setSectionResizeMode(ContactModel::ColMobile,   QHeaderView::ResizeToContents);
    hh->setSectionResizeMode(ContactModel::ColEmail,    QHeaderView::Stretch);
    hh->setSectionResizeMode(ContactModel::ColBirthday, QHeaderView::ResizeToContents);
    hh->setHighlightSections(false);

    ui->tableView->verticalHeader()->hide();
    ui->tableView->setFocusPolicy(Qt::StrongFocus);
    ui->tableView->setTabKeyNavigation(false);

    // Style sheet for a clean, modern look.
    setStyleSheet(QStringLiteral(
        "QTableView {"
        "  border: 1px solid #d0d0d0;"
        "  border-radius: 4px;"
        "  gridline-color: transparent;"
        "  font-size: 13px;"
        "}"
        "QTableView::item { padding: 6px 8px; }"
        "QTableView::item:selected { background: #0078d4; color: white; }"
        "QHeaderView::section {"
        "  background: #f5f5f5;"
        "  border: none;"
        "  border-bottom: 1px solid #d0d0d0;"
        "  padding: 6px 8px;"
        "  font-weight: bold;"
        "}"
        "QLineEdit#lineEditSearch {"
        "  border: 1px solid #d0d0d0;"
        "  border-radius: 4px;"
        "  padding: 4px 8px;"
        "  font-size: 13px;"
        "}"
        "QToolBar { spacing: 4px; padding: 4px; }"
    ));

    setupActions();
    setupSearchBar();

    connect(ui->tableView->selectionModel(),
            &QItemSelectionModel::selectionChanged,
            this, &MainWindow::onSelectionChanged);

    connect(ui->tableView, &QTableView::doubleClicked,
            this, &MainWindow::onDoubleClicked);

    onSelectionChanged(); // initialise action enable-states
    updateStatusLabel();
}

// ---------------------------------------------------------------------------
void MainWindow::setupActions()
{
    auto *actAdd = new QAction(QIcon::fromTheme(QStringLiteral("list-add")),
                               tr("&Add Contact"), this);
    actAdd->setShortcut(QKeySequence::New);
    actAdd->setStatusTip(tr("Add a new contact"));
    connect(actAdd, &QAction::triggered, this, &MainWindow::onAddContact);

    auto *actEdit = new QAction(QIcon::fromTheme(QStringLiteral("document-edit")),
                                tr("&Edit Contact"), this);
    actEdit->setObjectName(QStringLiteral("actionEdit"));
    actEdit->setShortcut(QKeySequence(Qt::Key_Return));
    actEdit->setStatusTip(tr("Edit the selected contact"));
    connect(actEdit, &QAction::triggered, this, &MainWindow::onEditContact);

    auto *actDelete = new QAction(QIcon::fromTheme(QStringLiteral("list-remove")),
                                  tr("&Delete Contact"), this);
    actDelete->setObjectName(QStringLiteral("actionDelete"));
    actDelete->setShortcut(QKeySequence::Delete);
    actDelete->setStatusTip(tr("Delete the selected contact"));
    connect(actDelete, &QAction::triggered, this, &MainWindow::onDeleteContact);

    auto *actQuit = new QAction(tr("&Quit"), this);
    actQuit->setShortcut(QKeySequence::Quit);
    connect(actQuit, &QAction::triggered, this, &QWidget::close);

    ui->menuContacts->addAction(actAdd);
    ui->menuContacts->addAction(actEdit);
    ui->menuContacts->addAction(actDelete);
    ui->menuContacts->addSeparator();
    ui->menuContacts->addAction(actQuit);

    ui->mainToolBar->addAction(actAdd);
    ui->mainToolBar->addAction(actEdit);
    ui->mainToolBar->addSeparator();
    ui->mainToolBar->addAction(actDelete);
}

void MainWindow::setupSearchBar()
{
    connect(ui->lineEditSearch, &QLineEdit::textChanged,
            this, &MainWindow::onSearchTextChanged);
}

// ---------------------------------------------------------------------------
void MainWindow::updateStatusLabel()
{
    const int total    = m_model->rowCount();
    const int visible  = m_proxy->rowCount();
    if (visible == total)
        ui->labelStatus->setText(tr("%n contact(s)", nullptr, total));
    else
        ui->labelStatus->setText(tr("%1 of %2 contacts").arg(visible).arg(total));
}

// ---------------------------------------------------------------------------
// Slots
// ---------------------------------------------------------------------------
void MainWindow::onAddContact()
{
    ContactDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    Contact c = dlg.contact();
    if (!m_model->addContact(c)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not save contact:\n%1")
                                 .arg(m_model->lastError()));
    }
    updateStatusLabel();
}

void MainWindow::onEditContact()
{
    const int row = selectedRow();
    if (row < 0) return;

    // Map proxy row → source row.
    const int sourceRow = m_proxy->mapToSource(m_proxy->index(row, 0)).row();
    const Contact original = m_model->contactAt(sourceRow);

    ContactDialog dlg(this, original);
    if (dlg.exec() != QDialog::Accepted) return;

    Contact updated = dlg.contact();
    updated.id = original.id; // preserve PK

    if (!m_model->updateContact(sourceRow, updated)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not update contact:\n%1")
                                 .arg(m_model->lastError()));
    }
}

void MainWindow::onDeleteContact()
{
    const int row = selectedRow();
    if (row < 0) return;

    const int sourceRow = m_proxy->mapToSource(m_proxy->index(row, 0)).row();
    const Contact c = m_model->contactAt(sourceRow);

    const auto btn = QMessageBox::question(
        this,
        tr("Delete Contact"),
        tr("Delete \"%1\"? This cannot be undone.").arg(c.name),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (btn != QMessageBox::Yes) return;

    if (!m_model->deleteContact(sourceRow)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not delete contact:\n%1")
                                 .arg(m_model->lastError()));
    }
    updateStatusLabel();
}

void MainWindow::onSelectionChanged()
{
    const bool hasSelection = selectedRow() >= 0;
    if (auto *a = findChild<QAction *>(QStringLiteral("actionEdit")))
        a->setEnabled(hasSelection);
    if (auto *a = findChild<QAction *>(QStringLiteral("actionDelete")))
        a->setEnabled(hasSelection);
}

void MainWindow::onSearchTextChanged(const QString &text)
{
    m_proxy->setFilterFixedString(text);
    updateStatusLabel();
}

void MainWindow::onDoubleClicked(const QModelIndex &/*index*/)
{
    onEditContact();
}

// ---------------------------------------------------------------------------
int MainWindow::selectedRow() const
{
    const QModelIndexList sel =
        ui->tableView->selectionModel()->selectedRows();
    return sel.isEmpty() ? -1 : sel.first().row();
}

// ---------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
