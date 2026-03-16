#pragma once

#include <QMainWindow>
#include "contactmodel.h"
#include "databasemanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief Application main window.
 *
 * Hosts a QTableView displaying all contacts and a toolbar/menu with
 * Add, Edit, and Delete actions.  The window owns both the DatabaseManager
 * and the ContactModel.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private slots:
    void onAddContact();
    void onEditContact();
    void onDeleteContact();
    void onSelectionChanged();
    void onSearchTextChanged(const QString &text);
    void onDoubleClicked(const QModelIndex &index);

private:
    void setupActions();
    void setupSearchBar();
    void updateStatusLabel();

    int selectedRow() const;

    Ui::MainWindow    *ui;
    DatabaseManager   *m_db;
    ContactModel      *m_model;
    class QSortFilterProxyModel *m_proxy;
};
