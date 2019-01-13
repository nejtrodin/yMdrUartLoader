//#include <QGuiApplication>
#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QThread>
#include <QDebug>

#include "uartloader.h"
#include "fileiohelper.h"


int main(int argc, char *argv[])
{
  QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

//  QGuiApplication app(argc, argv);  // for qml dialogs
  QApplication app(argc, argv);  // for native file dialogs

  app.setWindowIcon(QIcon("://images/yMdrUartLoader-48x48.png"));
  qmlRegisterType<UartLoader>("com.mycompany.loader", 1, 0, "UartLoader");

  UartLoader *loader = new UartLoader;
  FileIoHelper *fileIoHelper = new FileIoHelper;

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("loader", loader);
  engine.rootContext()->setContextProperty("fileIoHelper", fileIoHelper);
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

  return app.exec();
}
