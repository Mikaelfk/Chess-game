#include "AndroidGame.h"
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QStandardPaths>

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("Mikaelfk");
    QCoreApplication::setApplicationName("Chess");
    QQuickStyle::setStyle("Material");
    AndroidGame game(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/game.json");
    QObject::connect(&app, &QGuiApplication::applicationStateChanged, &game, [&game](Qt::ApplicationState state) {
        game.setApplicationActive(state == Qt::ApplicationActive);
    });
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("game", &game);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.load(QUrl("qrc:/mobile/Main.qml"));
    game.setApplicationActive(app.applicationState() == Qt::ApplicationActive);
    return app.exec();
}
