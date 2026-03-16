@echo off
:: ============================================================================
:: setup_git.bat  –  Initialise the repository with a clean commit history.
:: Run ONCE after creating the project folder.
::
:: Prerequisites: Git for Windows  (https://git-scm.com/download/win)
:: ============================================================================

:: ---- EDIT THESE BEFORE RUNNING ---------------------------------------------
set GIT_EMAIL=you@example.com
set GIT_NAME=Your Name
:: ----------------------------------------------------------------------------

cd /d "%~dp0"

git init
git config user.email "%GIT_EMAIL%"
git config user.name  "%GIT_NAME%"

:: Commit 1 – scaffold
git add .gitignore addressbook.pro build.bat build.sh LICENSE
git commit -m "chore: initial project scaffold"

:: Commit 2 – data model
git add src\contact.h
git commit -m "feat: add Contact plain-struct data model"

:: Commit 3 – validators
git add src\validator.h src\validator.cpp
git commit -m "feat: add field-level Validator namespace"

:: Commit 4 – database
git add src\databasemanager.h src\databasemanager.cpp
git commit -m "feat: add DatabaseManager with SQLite and schema migrations"

:: Commit 5 – model
git add src\contactmodel.h src\contactmodel.cpp
git commit -m "feat: add ContactModel (QAbstractTableModel)"

:: Commit 6 – dialog
git add src\contactdialog.h src\contactdialog.cpp src\contactdialog.ui
git commit -m "feat: add ContactDialog for Add/Edit operations"

:: Commit 7 – main window
git add src\mainwindow.h src\mainwindow.cpp src\mainwindow.ui src\main.cpp
git commit -m "feat: add MainWindow with QTableView, search, and toolbar"

:: Commit 8 – tests
git add tests\
git commit -m "test: add unit tests for Validator and DatabaseManager"

:: Commit 9 – CI
git add .github\
git commit -m "ci: add GitHub Actions for Linux and Windows builds"

:: Commit 10 – docs
git add README.md docs\
git commit -m "docs: add README and architecture documentation"

echo.
git log --oneline
echo.
echo Done. Next steps:
echo   1. Create a new repo on github.com
echo   2. git remote add origin https://github.com/^<YOU^>/addressbook.git
echo   3. git push -u origin main
