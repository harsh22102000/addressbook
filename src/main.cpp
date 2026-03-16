#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Application meta-data (used by QStandardPaths for data directory).
    app.setOrganizationName(QStringLiteral("AddressBook"));
    app.setApplicationName(QStringLiteral("AddressBook"));
    app.setApplicationVersion(QStringLiteral("1.0.0"));

    MainWindow window;
    window.show();

    return app.exec();
}
