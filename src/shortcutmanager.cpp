#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QShortcut>
#include <QStandardPaths>
#include <QSysInfo>
#include <QtWidgets/QWidget>

namespace ShortcutManager {

bool isShortcutPresent()
{
    QString shortcutName = "ActionPadServer.lnk";
    QString startupPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QDir::separator() + "Startup";
    QString shortcutPath = startupPath + QDir::separator() + shortcutName;
    return QFile::exists(shortcutPath);
}

void manageShortcut(bool state)
{
    QString shortcutName = "ActionPadServer.lnk";
    QString applicationPath = QCoreApplication::applicationFilePath();
    QString startupPath = QStandardPaths::writableLocation(QStandardPaths::ApplicationsLocation) + QDir::separator() + "Startup";
    QString shortcutPath = startupPath + QDir::separator() + shortcutName;

    if (state) {
        QFile::link(applicationPath, shortcutPath);
    } else {
        QFile::remove(shortcutPath);
    }
}

} // namespace ShortcutManager
