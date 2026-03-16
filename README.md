# Address Book

A clean, cross-platform address book application built with **Qt 6 / C++17** and **SQLite**.  
Runs on **Windows** and **Debian/Ubuntu Linux**, buildable entirely from the command line.

![CI](https://github.com/<YOUR_USERNAME>/addressbook/actions/workflows/ci.yml/badge.svg)

---

## Features

| Feature | Details |
|---|---|
| Contact fields | Name, Mobile, Email, Birthday |
| Persistence | SQLite database via Qt SQL module |
| Schema migrations | Versioned, idempotent migrations tracked in `schema_migrations` table |
| Validation | Per-field validators with inline error messages in the dialog |
| Search | Real-time, case-insensitive search across all columns |
| Sorting | Click any column header to sort ascending / descending |
| Unit tests | Qt Test – covers Validator (17 cases) and DatabaseManager (13 cases) |
| Build system | qmake (Qt 6) – `build.bat` on Windows, `build.sh` on Linux |
| CI | GitHub Actions – Linux **and** Windows build + test on every push/PR |

---

## Requirements

| | Windows | Linux (Debian/Ubuntu) |
|---|---|---|
| Qt | 6.2 or later | 6.2 or later |
| Compiler | MSVC 2019/2022 **or** MinGW-w64 | GCC / Clang (C++17) |
| Build tool | `nmake` (MSVC) **or** `mingw32-make` | `make` |
| SQLite | bundled with Qt | bundled with Qt |

---

## Windows – Quick Start

### Step 1 – Install Qt

1. Download the **Qt Online Installer** from <https://www.qt.io/download-qt-installer>
2. Sign in (free Qt account) and select **Qt 6.7.x → Desktop → MinGW 64-bit** (or MSVC 2022 64-bit)
3. Also install **Qt 6.7.x → Additional Libraries → Qt SQL** if not selected by default

### Step 2 – Open the correct command prompt

**MinGW (recommended – no extra setup):**
- Open **Qt 6.x.x (MinGW 64-bit)** from the Start menu  
  (it pre-sets PATH for `qmake` and `mingw32-make`)

**MSVC:**
- Open **Developer Command Prompt for VS 2022**, then run:
  ```
  "C:\Qt\6.7.0\msvc2022_64\bin\qtenv2.bat"
  ```

### Step 3 – Build and run

```bat
cd addressbook
build.bat --run
```

Other options:

```bat
build.bat             :: build only
build.bat --run       :: build + launch the app
build.bat --tests     :: build + run unit tests
build.bat --clean     :: delete build artefacts
```

> **Tip:** If `qmake` is not on your PATH, open `build.bat` in Notepad and set  
> `set QT_DIR=C:\Qt\6.7.0\mingw_64\bin`  (adjust to your actual install path)

### Step 4 – Deploy (distribute to other PCs)

After building, run `windeployqt` to collect the required Qt DLLs:

```bat
cd build\release
windeployqt addressbook.exe
```

The `release\` folder can then be zipped and shared — no Qt installation needed on the target PC.

---

## Linux (Debian / Ubuntu) – Quick Start

### Step 1 – Install dependencies

```bash
sudo apt-get update
sudo apt-get install \
    qt6-base-dev qt6-base-dev-tools \
    libqt6sql6-sqlite \
    build-essential libgl1-mesa-dev
```

### Step 2 – Build and run

```bash
chmod +x build.sh
./build.sh --run
```

Other options:

```bash
./build.sh            # build only
./build.sh --run      # build + launch the app
./build.sh --tests    # build + run unit tests
./build.sh --clean    # delete build artefacts
```

---

## Running Unit Tests

**Windows:**
```bat
build.bat --tests
```

**Linux:**
```bash
./build.sh --tests
```

**Manual (both platforms):**
```
cd tests
mkdir build && cd build
qmake ../tests.pro
<make>                        # nmake / mingw32-make / make
./addressbook_tests -v2       # Linux
release\addressbook_tests.exe -v2   # Windows
```

### Test coverage

| Suite | Cases | What is tested |
|---|---|---|
| `TestValidator` | 17 | Name, mobile, email, birthday – valid and invalid inputs, boundary values |
| `TestDatabaseManager` | 13 | Open/close, add, update, delete, sort order, unknown-id safety, migration idempotency |

---

## Project Structure

```
addressbook/
├── addressbook.pro              # qmake project (main app)
├── build.bat                    # Windows build helper
├── build.sh                     # Linux build helper
├── src/
│   ├── main.cpp                 # Entry point
│   ├── contact.h                # Plain data struct (no Qt dependency)
│   ├── validator.h/.cpp         # Field-level validation (pure functions)
│   ├── databasemanager.h/.cpp   # SQLite persistence + schema migrations
│   ├── contactmodel.h/.cpp      # QAbstractTableModel wrapping the DB
│   ├── mainwindow.h/.cpp/.ui    # Main window
│   └── contactdialog.h/.cpp/.ui # Add / Edit dialog
├── tests/
│   ├── tests.pro                # qmake project (unit tests)
│   ├── tst_validator.cpp        # Validator unit tests
│   └── tst_databasemanager.cpp  # DatabaseManager integration tests
├── docs/
│   └── architecture.md          # Architecture deep-dive
└── .github/
    └── workflows/
        └── ci.yml               # GitHub Actions: Linux + Windows CI
```

---

## Architecture

```
┌──────────────────────────────────────┐
│            UI Layer                  │  MainWindow, ContactDialog
├──────────────────────────────────────┤
│           Model Layer                │  ContactModel (QAbstractTableModel)
│                                      │  + QSortFilterProxyModel (search/sort)
├──────────────────────────────────────┤
│         Validation Layer             │  Validator (pure free functions)
├──────────────────────────────────────┤
│        Persistence Layer             │  DatabaseManager (Qt SQL / SQLite)
├──────────────────────────────────────┤
│           Data Layer                 │  Contact (plain struct)
└──────────────────────────────────────┘
```

- **`Contact`** is a plain struct — zero Qt overhead, trivially copyable.
- **`Validator`** is a namespace of pure functions — no state, no mocking needed in tests.
- **`DatabaseManager`** owns all SQL — UI and model never write SQL directly.
- **`ContactModel`** owns the in-memory cache — emits standard Qt model signals.
- **Schema migrations** are numbered SQL blocks; a `schema_migrations` table tracks applied versions.

See [`docs/architecture.md`](docs/architecture.md) for a detailed walk-through.

---

## Database Schema

```sql
CREATE TABLE schema_migrations (
    version INTEGER PRIMARY KEY   -- tracks applied migrations
);

CREATE TABLE contacts (
    id       INTEGER PRIMARY KEY AUTOINCREMENT,
    name     TEXT    NOT NULL,
    mobile   TEXT    NOT NULL DEFAULT '',
    email    TEXT    NOT NULL DEFAULT '',
    birthday TEXT    NOT NULL DEFAULT ''   -- ISO-8601: yyyy-MM-dd
);
```

---

## Validation Rules

| Field | Rule |
|---|---|
| **Name** | Required. 1–100 characters. |
| **Mobile** | Optional. Digits, spaces, `+`, `-`, `(`, `)` only; 7–15 significant digits. |
| **Email** | Optional. Must match `local@domain.tld` pattern. |
| **Birthday** | Optional. Valid date, before today, year ≥ 1900. |

---

## Keyboard Shortcuts

| Action | Shortcut |
|---|---|
| Add contact | `Ctrl+N` |
| Edit contact | `Enter` or double-click row |
| Delete contact | `Delete` |
| Quit | `Ctrl+Q` |

---

## CI / GitHub Actions

Every push to `main`/`develop` and every pull request triggers:

1. **Linux job** – installs Qt6 on `ubuntu-latest`, builds app + tests, runs tests.
2. **Windows job** – installs Qt6 MinGW via `jurplel/install-qt-action`, builds app + tests, runs tests, and uploads `addressbook.exe` as a build artefact.

---

## Pushing to GitHub

```bash
# 1. Initialise git with clean commit history
bash setup_git.sh        # Linux
# (on Windows, run each git command in setup_git.sh manually)

# 2. Create a repo on github.com, then:
git remote add origin https://github.com/<YOU>/addressbook.git
git push -u origin main
```

---

## License

MIT – see [LICENSE](LICENSE).
