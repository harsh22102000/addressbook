QT += testlib sql core
QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

TARGET = addressbook_tests
TEMPLATE = app

# Pull in the modules under test directly (no separate library needed).
SOURCES += \
    tst_validator.cpp \
    tst_databasemanager.cpp \
    ../src/validator.cpp \
    ../src/databasemanager.cpp

HEADERS += \
    ../src/validator.h \
    ../src/databasemanager.h \
    ../src/contact.h

INCLUDEPATH += ../src
