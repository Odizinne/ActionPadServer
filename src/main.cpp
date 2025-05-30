#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");
    QApplication app(argc, argv);
    app.setOrganizationName("Odizinne");
    app.setApplicationName("ActionPadServer");
    app.setQuitOnLastWindowClosed(false);

    QQmlApplicationEngine engine;
    engine.loadFromModule("Odizinne.ActionPadServer", "Main");

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
