#ifndef UARTLOADER_H
#define UARTLOADER_H

#include <QtCore>

class UartLoader : public QObject
{
  Q_OBJECT
  Q_PROPERTY(QList<QVariant> portList READ portList)

public:
  UartLoader(QObject *parent = Q_NULLPTR);

  // function called from qml
  Q_INVOKABLE void setPortSettings(QString portName, qint32 baud);
  Q_INVOKABLE void getRevision();
  Q_INVOKABLE void writeLoader();
  Q_INVOKABLE void eraseFlash();
  Q_INVOKABLE void writeFlash(QString path);
  Q_INVOKABLE void verifyFlash(QString path);
  Q_INVOKABLE void runMcu();

  QList<QVariant> portList();

public slots:

signals:
  // for thread worker
  void signalSetPortSettins(QString portName, qint32 baud);
  void signalGetRevision();
  void signalWriteLoader();
  void signalEraseFlash();
  void signalWriteFlash(QString path);
  void signalVerifyFlash(QString path);
  void signalRunMcu();

  // for qml gui
  void signalWriteLog(QString line);
  void signalRevision(bool ok, QString revision);
  void signalLoaderWrited(bool ok);
  void signalFlashErased(bool ok);
  void signalFlashWrited(bool ok);
  void signalFlashVerified(bool ok);
  void signalMcuRuned(bool ok);
};

#endif // UARTLOADER_H
