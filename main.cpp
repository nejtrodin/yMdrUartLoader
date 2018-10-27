#include <QGuiApplication>
#include <QQuickItem>
#include <QQuickView>
#include <QQmlEngine>
#include <QQmlContext>
#include <QThread>
#include <QDebug>

#include "uartloader.h"


int main(int argc, char *argv[])
{
  QGuiApplication app(argc, argv);
  app.setWindowIcon(QIcon("://images/yMdrUartLoader-48x48.png"));
  qmlRegisterType<UartLoader>("com.mycompany.loader", 1, 0, "UartLoader");

  UartLoader *loader = new UartLoader;
  QQuickView view;
  view.engine()->rootContext()->setContextProperty("loader", loader);
  view.setSource(QUrl(QLatin1String("qrc:/main.qml")));
  view.setResizeMode(QQuickView::SizeRootObjectToView);

  view.show();
  return app.exec();
}
