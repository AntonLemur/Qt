#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "ordermodel.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;

    // Передаем объект в контекст QML под именем "deliveryModel"
    OrderModel model;
    engine.rootContext()->setContextProperty("deliveryModel", &model);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("courierapp", "Main");

    return app.exec();
}
