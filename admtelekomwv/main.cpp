#include <QGuiApplication>
#include <QQmlApplicationEngine>
// #include <QtWebView>
#include "thumbnailprovider.h"

int main(int argc, char *argv[])
{
    // QtWebView::initialize();

    QGuiApplication app(argc, argv);

    // 1. Load dependencies in the correct order via the Java layer
    // This will automatically call JNI_OnLoad and install s_jvm in VLC
    QJniObject::callStaticMethod<void>(
        "java/lang/System",
        "loadLibrary",
        "(Ljava/lang/String;)V",
        QJniObject::fromString("vlc").object() // core first
        );

    // QJniObject::callStaticMethod<void>(
    //     "java/lang/System",
    //     "loadLibrary",
    //     "(Ljava/lang/String;)V",
    //     QJniObject::fromString("vlcjni").object() // then use the bridge with JNI_OnLoad
    //     );

    QQmlApplicationEngine engine;

    // Регистрация: "thumb" — это имя провайдера
    engine.addImageProvider(QLatin1String("thumb"), new ThumbnailProvider);

    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("admtelekomwv", "Main");

    return app.exec();
}
