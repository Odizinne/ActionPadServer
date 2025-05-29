#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "actionpadserver.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qmlRegisterType<ActionPadServer>("Odizinne.ActionPadServer", 1, 0, "ActionPadServer");
    qmlRegisterType<ActionModel>("Odizinne.ActionPadServer", 1, 0, "ActionModel");

    QQmlApplicationEngine engine;
    engine.loadFromModule("Odizinne.ActionPadServer", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
