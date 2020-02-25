#ifndef UARTLOADERWORKER_H
#define UARTLOADERWORKER_H

// в этот класс переносятся функции работающие в отдельном потоке

#define MAX_CODE_SIZE  0x20000  // 128k
#define MAX_RAM_SIZE   0x8000  // 32k

#include <QtCore>

class QSerialPort;

class UartLoaderWorker : public QObject
{
  Q_OBJECT
public:
  UartLoaderWorker(QObject *parent = Q_NULLPTR);

public slots:
  void slotSetSettings(QString strMcuType, QString portName, qint32 baud);
  void slotGetRevision();
  void slotWriteLoader();
  void slotEraseFlash();
  void slotWriteFlash(QString path);
  void slotVerifyFlash(QString path);
  void slotRunMcu();

signals:
  void signalWriteLog(QString line);
  void signalRevision(bool ok, QString revision);
  void signalLoaderWrited(bool ok);
  void signalFlashErased(bool ok);
  void signalFlashWrited(bool ok);
  void signalFlashVerified(bool ok);
  void signalMcuRuned(bool ok);

private:
  bool loadHexCode(QString path);
  bool loadHexLoader(QString path);
  void identifyOfMode();
  QByteArray readSerialData(int size, int timeout);

  QString m_lastError;

  enum {
    None,
    BareMode,
    RamMode,
    PortBusy
  } mode;

  enum {
    Type1986VE1T,
    Type1986VE9x
  } mcuType;

  // code in flash
  /*  m_flashStartAddress
   *  v
   *  [ FLASH = MAX_CODE_SIZE ]
   *  [ CODE ]
   *         ^
   *         m_flashLength + m_flashStartAddress
   */
  quint8 m_flashBuf[MAX_CODE_SIZE];
  quint32 m_flashStartAddress;
  quint32 m_flashLength;

  // loader in ram
  /*  m_ramStartAddress
   *  v
   *  [ RAM = MAX_RAM_SIZE ]
   *     [ LOADER ]
   *     ^        ^
   *     |        m_ramLength + m_ramStartOffset
   *     m_ramStartOffset
   */
  quint8 m_ramBuf[MAX_RAM_SIZE];
  quint32 m_ramStartAddress;
  quint32 m_ramLength;
  quint32 m_ramStartOffset;

  //serial port
  QString m_portName;
  qint32 m_baud;
  QSerialPort *m_serialPort;

  const char m_idString[13]= {'1', '9', '8', '6', 'B', 'O', 'O', 'T', 'U', 'A', 'R', 'T'};

};

#endif // UARTLOADERWORKER_H
