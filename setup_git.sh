#!/usr/bin/env bash
# setup_git.sh – initialise the repository with a clean, meaningful commit history.
# Run this ONCE after cloning / creating the repo directory.
# Usage: bash setup_git.sh
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

git init
git config user.email "you@example.com"   # replace before pushing
git config user.name  "Your Name"          # replace before pushing

# ── Commit 1: project scaffold ──────────────────────────────────────────────
git add .gitignore addressbook.pro build.sh LICENSE
git commit -m "chore: initial project scaffold

Add qmake project file, .gitignore, MIT licence, and build.sh helper.
The helper supports --run, --tests, and --clean flags."

# ── Commit 2: Contact data model ────────────────────────────────────────────
git add src/contact.h
git commit -m "feat: add Contact plain-struct data model

Introduce a trivially-copyable Contact struct with id, name, mobile,
email, and birthday fields.  id == -1 signals an unpersisted record."

# ── Commit 3: field validators ──────────────────────────────────────────────
git add src/validator.h src/validator.cpp
git commit -m "feat: add field-level Validator namespace

Pure functions returning an empty string (valid) or a human-readable
error message (invalid).  Covers:
  - Name: required, 1-100 chars
  - Mobile: optional; digits/+/-/spaces only, 7-15 significant digits
  - Email: optional; RFC-5322 simplified pattern
  - Birthday: optional; past date, >= 1900"

# ── Commit 4: database layer ─────────────────────────────────────────────────
git add src/databasemanager.h src/databasemanager.cpp
git commit -m "feat: add DatabaseManager with SQLite persistence

Wraps QSqlDatabase (SQLite) with:
  - WAL journal mode for better concurrency
  - Versioned schema migrations tracked in schema_migrations table
  - Full CRUD for Contact records
  - Contacts returned sorted by name (COLLATE NOCASE)

Migration 1 creates the contacts table."

# ── Commit 5: table model ────────────────────────────────────────────────────
git add src/contactmodel.h src/contactmodel.cpp
git commit -m "feat: add ContactModel (QAbstractTableModel)

Provides a model layer between DatabaseManager and the table view.
Maintains an in-memory sorted cache; emits standard Qt model signals
so QTableView and QSortFilterProxyModel react automatically."

# ── Commit 6: contact dialog ─────────────────────────────────────────────────
git add src/contactdialog.h src/contactdialog.cpp src/contactdialog.ui
git commit -m "feat: add ContactDialog for Add/Edit operations

Modal dialog with form fields for all Contact fields.
  - QDateEdit uses a sentinel date (1900-01-01) for 'not set'
  - Runs Validator on Accept; shows inline error label on failure
  - Pre-populates fields when editing an existing contact"

# ── Commit 7: main window ────────────────────────────────────────────────────
git add src/mainwindow.h src/mainwindow.cpp src/mainwindow.ui src/main.cpp
git commit -m "feat: add MainWindow with QTableView and toolbar actions

  - Displays all contacts in a sortable, searchable QTableView
  - Add (Ctrl+N), Edit (Enter / double-click), Delete (Del) actions
  - QSortFilterProxyModel provides live search across all columns
  - Status label shows 'N contacts' / 'X of N contacts' when filtering
  - Edit/Delete actions disabled when no row is selected"

# ── Commit 8: unit tests ─────────────────────────────────────────────────────
git add tests/
git commit -m "test: add unit tests for Validator and DatabaseManager

TestValidator  – 17 cases covering all validation rules including
                 boundary conditions and optional-field semantics.
TestDatabaseManager – 13 cases covering open/close, CRUD, sort order,
                 edge cases (unknown id), and migration idempotency.

Build: cd tests && qmake6 tests.pro && make && ./addressbook_tests -v2"

# ── Commit 9: CI ─────────────────────────────────────────────────────────────
git add .github/
git commit -m "ci: add GitHub Actions workflow (build + test)

Triggered on push to main/develop and on pull requests to main.
Steps: install Qt6, build app, build tests, run tests with -v2."

# ── Commit 10: documentation ─────────────────────────────────────────────────
git add README.md docs/
git commit -m "docs: add README and architecture documentation

README covers: features, requirements, quick-start, build instructions,
project structure, architecture overview, DB schema, validation rules,
keyboard shortcuts, CI badge, and contributing guide.

docs/architecture.md provides a component diagram, data-flow walkthrough,
migration strategy explanation, and thread-safety notes."

echo ""
echo "Git history initialised.  $(git log --oneline | wc -l) commits."
echo ""
echo "Next steps:"
echo "  1. Replace user.email and user.name in this script (or via git config)."
echo "  2. Create a GitHub repository."
echo "  3. git remote add origin https://github.com/<YOU>/addressbook.git"
echo "  4. git push -u origin main"
