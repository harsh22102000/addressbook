QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

TARGET   = addressbook
TEMPLATE = app

# ── Source files ─────────────────────────────────────────────────────────────
SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/contactdialog.cpp \
    src/contactmodel.cpp \
    src/databasemanager.cpp \
    src/validator.cpp

HEADERS += \
    src/mainwindow.h \
    src/contactdialog.h \
    src/contactmodel.h \
    src/databasemanager.h \
    src/contact.h \
    src/validator.h

FORMS += \
    src/mainwindow.ui \
    src/contactdialog.ui

# ── Windows-specific ─────────────────────────────────────────────────────────
win32 {
    VERSION = 1.0.0.0
    QMAKE_TARGET_COMPANY     = "Address Book"
    QMAKE_TARGET_PRODUCT     = "Address Book"
    QMAKE_TARGET_DESCRIPTION = "Qt Address Book Application"
    QMAKE_TARGET_COPYRIGHT   = "MIT"
    CONFIG += windows
}

# ── Linux/macOS deployment ────────────────────────────────────────────────────
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
