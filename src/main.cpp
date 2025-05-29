#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "actionpadserver.h"

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");
    QGuiApplication app(argc, argv);
    app.setOrganizationName("Odizinne");
    app.setApplicationName("ActionPadServer");

    qmlRegisterType<ActionPadServer>("Odizinne.ActionPadServer", 1, 0, "ActionPadServer");
    qmlRegisterType<ActionModel>("Odizinne.ActionPadServer", 1, 0, "ActionModel");

    QQmlApplicationEngine engine;
    engine.loadFromModule("Odizinne.ActionPadServer", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
