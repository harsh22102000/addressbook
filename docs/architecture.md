# Architecture

This document provides a deeper look at the internal design of the Address Book application.

---

## Component Diagram

```
main.cpp
  └── MainWindow  (QMainWindow)
        ├── DatabaseManager  (QObject)
        │     └── QSqlDatabase  [SQLite, WAL mode]
        ├── ContactModel  (QAbstractTableModel)
        │     └── holds: QVector<Contact>
        ├── QSortFilterProxyModel
        │     └── wraps ContactModel (search + sort)
        └── ContactDialog  (QDialog)
              └── Validator namespace
```

---

## Data Flow – Adding a Contact

```
User fills ContactDialog
        │
        ▼
Validator::validate*()     ← pure functions, no side effects
        │  (error → show inline message, stay open)
        ▼
ContactModel::addContact(Contact)
        │
        ├── DatabaseManager::addContact(Contact &)
        │       └── INSERT INTO contacts … → sets contact.id
        │
        └── beginInsertRows / endInsertRows
                └── QSortFilterProxyModel notified
                        └── QTableView redraws
```

---

## Schema Migration Strategy

The approach is deliberately simple and inspired by Rails migrations and Alembic:

- A `schema_migrations` table stores the integer version of every applied migration.
- On startup, `DatabaseManager::applyMigrations()` iterates `kMigrations` (a compile-time list of `{version, SQL}` pairs) and applies any that are not yet recorded.
- Each migration runs inside a transaction; a failure rolls back and prevents the app from opening.
- **Adding a migration** = append one entry to `kMigrations`. Existing installs skip already-applied versions.

This is intentionally not a full Alembic/SQLAlchemy solution (which would require Python), but it follows the same conceptual model.

---

## Validator Design

`Validator` is a **namespace** of free functions that return `QString`:

- Empty string → valid
- Non-empty string → human-readable error message

This makes the validators:
- **Trivially testable** – no mocking, no fixtures, just call and `QVERIFY`.
- **Reusable** – can be called from the dialog, from tests, or from a hypothetical REST API layer without any coupling.
- **Stateless** – no hidden dependencies or side effects.

---

## Model / View Separation

```
QTableView  (view)
      │  reads display data via QAbstractItemModel interface
      ▼
QSortFilterProxyModel  (proxy)
      │  delegates to source model; adds search + sort
      ▼
ContactModel  (source model)
      │  owns QVector<Contact> cache
      │  delegates persistence to…
      ▼
DatabaseManager
      │  owns QSqlDatabase
      ▼
SQLite on disk
```

The UI (MainWindow, ContactDialog) never touches SQL directly.  
The model never touches Qt Widgets.  
Each layer can be tested independently.

---

## Thread Safety

The current implementation runs everything on the main (GUI) thread, which is appropriate for a desktop address book with small datasets. If the contact list grew large (> 10 000 rows), the database operations could be moved to a `QThread` + `QObject::moveToThread` worker, exposing signals for `contactsLoaded` etc., without changing the public API of `ContactModel`.
