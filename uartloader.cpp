#include "uartloader.h"

#include <QThread>
#include <QDebug>
//#include <QSerialPortInfo>
#include "qserialportinfo.h"
#include "uartloaderworker.h"

UartLoader::UartLoader(QObject *parent)
  : QObject(parent)
{
  QThread *thread = new QThread;
  UartLoaderWorker *worker = new UartLoaderWorker;
  worker->moveToThread(thread);
  connect(this, &UartLoader::signalSetPortSettins, worker, &UartLoaderWorker::slotSetPortSettings);
  connect(worker, &UartLoaderWorker::signalWriteLog, this, &UartLoader::signalWriteLog);
  connect(this, &UartLoader::signalGetRevision, worker, &UartLoaderWorker::slotGetRevision);
  connect(worker, &UartLoaderWorker::signalRevision, this, &UartLoader::signalRevision);
  connect(this, &UartLoader::signalWriteLoader, worker, &UartLoaderWorker::slotWriteLoader);
  connect(worker, &UartLoaderWorker::signalLoaderWrited, this, &UartLoader::signalLoaderWrited);
  connect(this, &UartLoader::signalEraseFlash, worker, &UartLoaderWorker::slotEraseFlash);
  connect(worker, &UartLoaderWorker::signalFlashErased, this, &UartLoader::signalFlashErased);
  connect(this, &UartLoader::signalWriteFlash, worker, &UartLoaderWorker::slotWriteFlash);
  connect(worker, &UartLoaderWorker::signalFlashWrited, this, &UartLoader::signalFlashWrited);
  connect(this, &UartLoader::signalVerifyFlash, worker, &UartLoaderWorker::slotVerifyFlash);
  connect(worker, &UartLoaderWorker::signalFlashVerified, this, &UartLoader::signalFlashVerified);
  connect(this, &UartLoader::signalRunMcu, worker, &UartLoaderWorker::slotRunMcu);
  connect(worker, &UartLoaderWorker::signalMcuRuned, this, &UartLoader::signalMcuRuned);
  thread->start();
}


void UartLoader::setPortSettings(QString portName, qint32 baud)
{
  emit signalSetPortSettins(portName, baud);
}


void UartLoader::getRevision()
{
  emit signalGetRevision();
}


void UartLoader::writeLoader()
{
  emit signalWriteLoader();
}


void UartLoader::eraseFlash()
{
  emit signalEraseFlash();
}


void UartLoader::writeFlash(QString path)
{
  emit signalWriteFlash(path);
}


void UartLoader::verifyFlash(QString path)
{
  emit signalVerifyFlash(path);
}


void UartLoader::runMcu()
{
  emit signalRunMcu();
}


QList<QVariant> UartLoader::portList()
{
  QList<QVariant> portList;
  const auto infos = QSerialPortInfo::availablePorts();
  for (const QSerialPortInfo &info : infos)
    {
      portList << info.systemLocation();

    }
  return portList;
}


