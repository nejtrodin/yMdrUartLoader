#include "uartloaderworker.h"

#include <QString>
#include <QFile>
#include <QByteArray>
//#include <QSerialPort>
#include "qserialport.h"
#include <QDebug>

const quint32 MDR1T_FLASH_START_ADDRESS = 0x00000000;
const quint32 MDR9X_FLASH_START_ADDRESS =  0x08000000;
const quint32 MDR1T_RAM_START_ADDRESS =   0x20100000;
const quint32 MDR9X_RAM_START_ADDRESS =    0x20000000;

UartLoaderWorker::UartLoaderWorker(QObject *parent) : QObject(parent)
{
  m_flashLength = 0;
  m_ramLength = 0;
  m_ramStartOffset = 0;
  mode = None;
  mcuType = Type1986VE1T;

  m_flashStartAddress = 0;
  m_ramStartAddress = 0;
  if (mcuType == Type1986VE1T)
    {
      m_flashStartAddress = MDR1T_FLASH_START_ADDRESS;
      m_ramStartAddress = MDR1T_RAM_START_ADDRESS;
    }
  else if (mcuType == Type1986VE9x)
    {
      m_flashStartAddress = MDR9X_FLASH_START_ADDRESS;
      m_ramStartAddress = MDR9X_RAM_START_ADDRESS;
    }

  m_serialPort = new QSerialPort;
}

void UartLoaderWorker::slotSetPortSettings(QString portName, qint32 baud)
{
  qDebug() << "Set Port Settings: "  << portName << ", " << baud;
  m_portName = portName;
  m_baud = baud;
}

void UartLoaderWorker::slotGetRevision()
{
  identifyOfMode();

  if (mode == PortBusy)
    {
      emit signalRevision(false, "");
      return;
    }

  if (mode == BareMode)
    {
      m_serialPort->setPortName(m_portName);
      m_serialPort->setBaudRate(m_baud);

      if (!m_serialPort->open(QIODevice::ReadWrite))
        {
          qDebug() << "slotGetRevision: port not opened";
          emit signalRevision(false, "");
          emit signalWriteLog(tr("Port %1 not opened").arg(m_portName));
          emit signalWriteLog(m_serialPort->errorString()); // !
          return;
        }
      m_serialPort->flush();

      QByteArray revisionCommand;
      revisionCommand.append(static_cast<char>(0x59));
      revisionCommand.append(static_cast<char>(0xff));
      revisionCommand.append(static_cast<char>(0x0f));
      revisionCommand.append(static_cast<char>(0x00));
      revisionCommand.append(static_cast<char>(0x00));
      revisionCommand.append(static_cast<char>(0x01));
      revisionCommand.append(static_cast<char>(0x00));
      revisionCommand.append(static_cast<char>(0x00));
      revisionCommand.append(static_cast<char>(0x00));

      m_serialPort->write(revisionCommand);
      m_serialPort->flush();

      QByteArray readData;
      readData = readSerialData(3, 1000);

      char ret = 0;
      if (readData.size() == 3 &&
          readData.at(0) == 0x59 &&
          readData.at(2) == 0x4b)
        {
          ret = readData.at(1);

          QString revision;
          switch (ret) {
            case static_cast<char>(0x00):
              revision.append(tr("Revision: 1"));
              break;
            case static_cast<char>(0x8d):
              revision.append(tr("Revision: 2"));
              break;
            case static_cast<char>(0x4d):
              revision.append(tr("Revision: 3"));
              break;
            case static_cast<char>(0xcd):
              revision.append(tr("Revision: 4"));
              break;
            case static_cast<char>(0x2d):
              revision.append(tr("Revision: 5"));
              break;
            case static_cast<char>(0xad):
              revision.append(tr("Revision: 6"));
              break;
            default:
              revision.append(tr("Revision unknown: 0x%1").arg(static_cast<int>(ret), 2, 16));
              break;
            }

          emit signalRevision(true, revision);
          emit signalWriteLog(revision);
        }
      else
        {
          emit signalRevision(false, "");
          emit signalWriteLog(tr("Get revision failed"));
        }

      m_serialPort->close();
      return;
    }
  else
    {
      qDebug() << "slotGetRevision: mcu not in BareMode";
      emit signalRevision(false, "");
      if (mode != None)
        {
          emit signalWriteLog(tr("MCU not in Uart Loader Mode"));
          emit signalWriteLog(tr("Please restart MCU"));
        }
      return;
    }
}

void UartLoaderWorker::slotWriteLoader()
{
  identifyOfMode();

  if (mode == PortBusy)
    {
      emit signalLoaderWrited(false);
      return;
    }

  if (mode == BareMode)
    {
      QString loaderPath;
      if (mcuType == Type1986VE1T)
        loaderPath = ":/hexfiles/1986ve1bootuart.hex";
      else
        {
          qDebug() << "Mcu Type not defined";
          emit signalLoaderWrited(false);
          emit signalWriteLog(tr("Mcu Type not defined") + m_lastError);
          return;
        }

      if (!loadHexLoader(loaderPath))
        {
          qDebug() << m_lastError;
          emit signalLoaderWrited(false);
          emit signalWriteLog(tr("Bootloader file not opened. ") + m_lastError);
          return;
        }

      m_serialPort->setPortName(m_portName);
      m_serialPort->setBaudRate(m_baud);
      if (!m_serialPort->open(QIODevice::ReadWrite))
        {
          qDebug() << "slotWriteLoader: port not opened";
          emit signalLoaderWrited(false);
          emit signalWriteLog(tr("Port %1 not opened").arg(m_portName));
          return;
        }
      m_serialPort->flush();

      quint32 startAddress = m_ramStartAddress + m_ramStartOffset;
      QByteArray writeBuf;
      writeBuf.append('L');
      writeBuf.append(static_cast<char>(startAddress & 0xff));
      writeBuf.append(static_cast<char>((startAddress >> 8) & 0xff));
      writeBuf.append(static_cast<char>((startAddress >> 16) & 0xff));
      writeBuf.append(static_cast<char>((startAddress >> 24) & 0xff));
      writeBuf.append(static_cast<char>(m_ramLength & 0xff));
      writeBuf.append(static_cast<char>((m_ramLength >> 8) & 0xff));
      writeBuf.append(static_cast<char>((m_ramLength >> 16) & 0xff));
      writeBuf.append(static_cast<char>((m_ramLength >> 24) & 0xff));
      m_serialPort->write(writeBuf);
      m_serialPort->flush();

      QByteArray readBuf;
      readBuf = readSerialData(1, 1000);

      if (readBuf.length() < 1 ||
          readBuf.at(0) != 'L')
        {
          qDebug() << "slotWriteLoader: transmit error (1)";
          emit signalLoaderWrited(false);
          emit signalWriteLog(tr("Write bootloader failed: transmit error"));
          m_serialPort->close();
          return;
        }

      writeBuf.clear();
      for (uint i = 0; i < m_ramLength; i++)
        {
          writeBuf.append(static_cast<char>(m_ramBuf[m_ramStartOffset + i]));
        }
      m_serialPort->write(writeBuf);
      m_serialPort->flush();

      readBuf = readSerialData(1, 5000);
      if (readBuf.length() < 1 ||
          readBuf.at(0) != 'K')
        {
          qDebug() << "slotWriteLoader: transmit error (2)";
          emit signalLoaderWrited(false);
          emit signalWriteLog(tr("Write bootloader failed: transmit error"));
          m_serialPort->close();
          return;
        }

      for (uint i = 0; i < (m_ramLength >> 3); i++)
        {
          quint32 address = startAddress + 8*i;
          writeBuf.clear();
          writeBuf.append('Y');
          writeBuf.append(static_cast<char>(address & 0xff));
          writeBuf.append(static_cast<char>((address >> 8) & 0xff));
          writeBuf.append(static_cast<char>((address >> 16) & 0xff));
          writeBuf.append(static_cast<char>((address >> 24) & 0xff));
          writeBuf.append(static_cast<char>(8));
          writeBuf.append(static_cast<char>(0));
          writeBuf.append(static_cast<char>(0));
          writeBuf.append(static_cast<char>(0));
          m_serialPort->write(writeBuf);
          m_serialPort->flush();

          readBuf = readSerialData(10, 1000);
          if (readBuf.length() == 10 ||
              readBuf.at(0) == 'Y' ||
              readBuf.at(9) == 'K')
            {
              for (uint j = 0; j < 8; j++)
                {
                  quint8 readByte = static_cast<quint8>(readBuf.at(static_cast<int>(j)+1));
                  quint8 writeByte = m_ramBuf[m_ramStartOffset + 8*i + j];
                  if (readByte != writeByte)
                    {
                      qDebug() << "slotWriteLoader: transmit error (3)";
                      emit signalLoaderWrited(false);
                      emit signalWriteLog(tr("Write bootloader failed: transmit errorr"));
                      m_serialPort->close();
                      return;
                    }
                }
            }
          else
            {
              qDebug() << "slotWriteLoader: transmit error (4)";
              emit signalLoaderWrited(false);
              emit signalWriteLog(tr("Write bootloader failed: transmit error"));
              m_serialPort->close();
              return;
            }
        }

      // restart, run loader from RAM
      writeBuf.clear();
      writeBuf.append('R');
      writeBuf.append(static_cast<char>(m_ramBuf[m_ramStartOffset + 4] & 0xfe));
      writeBuf.append(static_cast<char>(m_ramBuf[m_ramStartOffset + 5]));
      writeBuf.append(static_cast<char>(m_ramBuf[m_ramStartOffset + 6]));
      writeBuf.append(static_cast<char>(m_ramBuf[m_ramStartOffset + 7]));
      m_serialPort->write(writeBuf);
      m_serialPort->flush();

      readBuf = readSerialData(1, 1000);
      if (readBuf.length() < 1 ||
          readBuf.at(0) != 'R')
        {
          qDebug() << "slotWriteLoader: transmit error (5)";
          emit signalLoaderWrited(false);
          emit signalWriteLog(tr("Write bootloader failed: transmit error"));
          m_serialPort->close();
          return;
        }

      // identify loader
      identifyOfMode();
      if (mode == RamMode)
        {
          qDebug() << "WriteLoader done";
          m_serialPort->close();
          emit signalLoaderWrited(true);
          emit signalWriteLog(tr("Write bootloader done"));
        }
      else
        {
          qDebug() << "slotWriteLoader: identify error (6)";
          m_serialPort->close();
          emit signalLoaderWrited(false);
          emit signalWriteLog(tr("Write bootloader failed"));
        }
    }
  else if (mode == RamMode)
    {
      emit signalLoaderWrited(true);
      emit signalWriteLog(tr("MCU already in Uart Loader Mode"));
    }
  else
    {
      qDebug() << "slotWriteLoader: mcu not in BareMode";
      emit signalLoaderWrited(false);
      if (mode != None)
        {
          emit signalWriteLog(tr("MCU not in Uart Loader Mode"));
          emit signalWriteLog(tr("Please restart MCU"));
        }
      return;
    }
}

void UartLoaderWorker::slotEraseFlash()
{
  identifyOfMode();

  if (mode == PortBusy)
    {
      emit signalFlashErased(false);
      return;
    }

  if (mode == BareMode)
    {
      slotWriteLoader();
      identifyOfMode();
    }

  if (mode == RamMode)
    {
      m_serialPort->setPortName(m_portName);
      m_serialPort->setBaudRate(m_baud);

      if (!m_serialPort->open(QIODevice::ReadWrite))
        {
          qDebug() << "erase: port not opened";
          emit signalFlashErased(false);
          emit signalWriteLog(tr("Port %1 not opened").arg(m_portName));
          return;
        }
      m_serialPort->flush();

      QByteArray writeBuf;
      writeBuf.append('E');
      m_serialPort->write(writeBuf);
      m_serialPort->flush();

      QByteArray readBuf;
      readBuf = readSerialData(9, 1000);

      if (readBuf.length() != 9)
        {
          qDebug() << "erase: error (1) - ошибка обмена";
          emit signalFlashErased(false);
          emit signalWriteLog(tr("Chip erase failed: transfer error"));
          m_serialPort->close();
          return;
        }

      quint32 address = static_cast<quint32> (
          (static_cast<quint8>(readBuf.at(1))) +
          (static_cast<quint8>(readBuf.at(2)) << 8) +
          (static_cast<quint8>(readBuf.at(3)) << 16) +
          (static_cast<quint8>(readBuf.at(4)) << 24));
      quint32 data = static_cast<quint32> (
          (static_cast<quint8>(readBuf.at(5))) +
          (static_cast<quint8>(readBuf.at(6)) << 8) +
          (static_cast<quint8>(readBuf.at(7)) << 16) +
          (static_cast<quint8>(readBuf.at(8)) << 24));

      if ((address == 0x000020000) && (data == 0xffffffff))
        {
          qDebug() << "Full chip erase done!";
          emit signalFlashErased(true);
          emit signalWriteLog("Chip erase done.");
          m_serialPort->close();
          return;
        }

      qDebug() << "Full chip erase failed adress=" << QString::number(address, 16)
               << " data=" << QString::number(data, 16);
      emit signalFlashErased(false);
      emit signalWriteLog("Chip erase failed: adress=" + QString::number(address, 16) +
                          " data=" + QString::number(data, 16));
      m_serialPort->close();
      return;
    }
  else
    {
      emit signalFlashErased(false);
      emit signalWriteLog(tr("Erase failed: MCU not in Ram mode"));
      return;
    }
}

void UartLoaderWorker::slotWriteFlash(QString path)
{
  identifyOfMode();

  if (mode == PortBusy)
    {
      emit signalFlashWrited(false);
      return;
    }

  if (mode == BareMode)
    {
      slotEraseFlash();  // erase with call slotWriteLoader
      identifyOfMode();
    }

  if (mode == RamMode)
    {
      if (!loadHexCode(path))
        {
          qDebug() << m_lastError;
          emit signalFlashWrited(false);
          emit signalWriteLog(tr("Can't open programm hex file. ") + m_lastError);
          return;
        }

      m_serialPort->setPortName(m_portName);
      m_serialPort->setBaudRate(m_baud);
      if (!m_serialPort->open(QIODevice::ReadWrite))
        {
          qDebug() << "slotWriteCode: port not opened";
          emit signalLoaderWrited(false);
          emit signalWriteLog(tr("Port %1 not opened").arg(m_portName));
          return;
        }
      m_serialPort->flush();

      // set address
      QByteArray writeBuf;
      writeBuf.append('A');
      writeBuf.append(static_cast<char>(0x00));
      writeBuf.append(static_cast<char>(0x00));
      writeBuf.append(static_cast<char>(0x00));
      writeBuf.append(static_cast<char>(0x00));
      m_serialPort->write(writeBuf);
      m_serialPort->flush();

      QByteArray readBuf;
      readBuf = readSerialData(1, 1000);
      if (readBuf.length() != 1 || (readBuf.at(0) != 0x00))
        {
          qDebug() << "slotWriteCode: error (1) - transmit error";
          emit signalFlashWrited(false);
          emit signalWriteLog(tr("Write programm failed: transmit error"));
          m_serialPort->close();
          return;
        }

      // start programm
      quint32 csum = 0;
      for (uint i = 0; i < (m_flashLength >> 8); i++)
        {
          writeBuf.clear();
          writeBuf.append('P');
          csum = 0;
          for (uint j = 0; j < 256; j++)
            {
              uint idx = (i << 8) + j;
              writeBuf.append(static_cast<char>(m_flashBuf[idx]));
              csum += m_flashBuf[idx];
            }
          m_serialPort->write(writeBuf);
          m_serialPort->flush();

          readBuf = readSerialData(1, 1000);
          if (readBuf.length() != 1 ||
              (readBuf.at(0) != static_cast<char>(csum & 0xff)))
            {
              qDebug() << "slotWriteCode: error (2) - transmit error";
              emit signalFlashWrited(false);
              emit signalWriteLog(tr("Write programm failed: transmit error"));
              m_serialPort->close();
              return;
            }
        }

      qDebug() << "Program " << m_flashLength << " byte done!";
      emit signalFlashWrited(true);
      emit signalWriteLog(tr("Program %1 byte done").arg(m_flashLength));
      m_serialPort->close();
      return;
    }
  else
    {
      emit signalFlashWrited(false);
      emit signalWriteLog(tr("Write programm failed: MCU not in Ram mode"));
      return;
    }
}

void UartLoaderWorker::slotVerifyFlash(QString path)
{
  identifyOfMode();

  if (mode == PortBusy)
    {
      emit signalFlashVerified(false);
      return;
    }

  if (mode == BareMode)
    {
      slotWriteLoader();
    }

  if (mode == RamMode)
    {
      if (!loadHexCode(path))
        {
          qDebug() << m_lastError;
          emit signalFlashVerified(false);
          emit signalWriteLog(tr("Can't open programm hex file. ") + m_lastError);
          return;
        }

      m_serialPort->setPortName(m_portName);
      m_serialPort->setBaudRate(m_baud);
      if (!m_serialPort->open(QIODevice::ReadWrite))
        {
          qDebug() << "slotVerify: port not opened";
          emit signalFlashVerified(false);
          emit signalWriteLog(tr("Port %1 not opened").arg(m_portName));
          return;
        }
      m_serialPort->flush();

      // set address
      QByteArray writeBuf;
      writeBuf.append('A');
      writeBuf.append(static_cast<char>(0x00));
      writeBuf.append(static_cast<char>(0x00));
      writeBuf.append(static_cast<char>(0x00));
      writeBuf.append(static_cast<char>(0x00));
      m_serialPort->write(writeBuf);
      m_serialPort->flush();

      QByteArray readBuf;
      readBuf = readSerialData(1, 1000);
      if (readBuf.size() != 1 || (readBuf.at(0) != 0x00))
        {
          qDebug() << "slotVerify: error (1) - transmit error";
          emit signalFlashVerified(false);
          emit signalWriteLog(tr("Verify programm failed: transmit error"));
          m_serialPort->close();
          return;
        }

      writeBuf.clear();
      writeBuf.append('V');
      for (uint i = 0; i < (m_flashLength >> 8); i++)
        {
          for (uint j = 0; j < 32; j++)
            {
              m_serialPort->write(writeBuf);
              m_serialPort->flush();
              readBuf = readSerialData(8, 1000);
              if (readBuf.length() != 8)
                {
                  qDebug() << "slotVerify: error (2) - transmit error";
                  emit signalFlashVerified(false);
                  emit signalWriteLog(tr("Verify programm failed: transmit error"));
                  m_serialPort->close();
                  return;
                }

              for (uint k = 0; k < 8; k++)
                {
                  if (static_cast<quint8>(readBuf.at(static_cast<int>(k))) !=
                                          m_flashBuf[k + (j << 3) + (i << 8)])
                    {
                      qDebug() << "Verify failed adr="
                               << QString::number(k + (j << 3) + (i << 8), 16)
                               << " dataw="
                               << QString::number(m_flashBuf[k + (j << 3) + (i << 8)], 16)
                               << " datar="
                               << QString::number(readBuf.at(static_cast<int>(k)), 16);
                      emit signalFlashVerified(false);
                      emit signalWriteLog(tr("Verify programm failed adress=%1 dataw=%2 datar=%3")
                                          .arg(k + (j << 3) + (i << 8), 16)
                                          .arg(m_flashBuf[k + (j << 3) + (i << 8)], 16)
                                          .arg(readBuf.at(static_cast<int>(k)), 16));
                      m_serialPort->close();
                      return;
                    }
                }

            }
        }

      qDebug() << "Verify programm done";
      emit signalFlashVerified(true);
      emit signalWriteLog(tr("Verify programm done"));
      m_serialPort->close();
      return;
    }
  else
    {
      emit signalFlashVerified(false);
      emit signalWriteLog(tr("Verify programm failed: MCU not in Ram mode"));
      return;
    }
}

void UartLoaderWorker::slotRunMcu()
{
  identifyOfMode();
  if (mode == BareMode)
    {
      slotWriteLoader();
      identifyOfMode();
    }

  if (mode == RamMode)
    {
      m_serialPort->setPortName(m_portName);
      m_serialPort->setBaudRate(m_baud);
      if (!m_serialPort->open(QIODevice::ReadWrite))
        {
          qDebug() << "slotRun: port not opened";
          emit signalMcuRuned(false);
          emit signalWriteLog(tr("Port %1 not opened").arg(m_portName));
          return;
        }
      m_serialPort->flush();

      // set address
      QByteArray writeBuf;
      writeBuf.append('R');

      m_serialPort->write(writeBuf);
      m_serialPort->flush();

      QByteArray readBuf;
      readBuf = readSerialData(1, 1000);
      if (readBuf.length() != 1 || (readBuf.at(0) != 'R'))
        {
          qDebug() << "slotRun: error (1) - transmit error";
          emit signalMcuRuned(false);
          emit signalWriteLog(tr("Run failed: transmit error"));
          m_serialPort->close();
          return;
        }

      qDebug() << "Run done";
      emit signalMcuRuned(true);
      emit signalWriteLog(tr("Run done"));
      m_serialPort->close();
      return;
    }
  else
    {
      emit signalMcuRuned(false);
      emit signalWriteLog(tr("Run failed: MCU not in Ram mode"));
      return;
    }
}

bool UartLoaderWorker::loadHexCode(QString path)
{
  QFile file(path);
  bool ok;
  quint32 sbaAddr = 0;  // адрес сегмента
  quint32 lbaAddr = 0;  // расширенный адрес

  for(uint i = 0; i < sizeof(m_flashBuf); i++)
    {
      m_flashBuf[i] = 0xff;
    }

  if (!file.open(QIODevice::ReadOnly))
    {
      m_lastError = tr("Hex file not opened");
      return false;
    }

  while(1)
    {
      int bLength = 0;  // length of note
      quint16 offset = 0;  // address of data
      int noteType = 0;  // type of note
      quint8 cSum = 0;  // checksum
      quint8 data_buf[255];  // buffer of reading data

      if (file.atEnd())
        {
          break;
        }

      QByteArray data;
      data = file.readLine();
      if (data.isEmpty())
        continue;

      // parse line
      if (data.at(0) != ':')
        continue;
      bLength = data.mid(1, 2).toInt(&ok, 16);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }
      offset = static_cast<quint16>(data.mid(3, 2).toInt(&ok, 16) << 8);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }
      offset += data.mid(5, 2).toInt(&ok, 16);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }
      noteType = data.mid(7, 2).toInt(&ok, 16);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }

      cSum = static_cast<quint8>(bLength + (offset >> 8) + (offset & 0xff) + noteType);

      if (bLength > (data.size() - 13))
        {
          m_lastError = tr("File corrupt. Line so short");
          file.close();
          return false;
        }

      for (int i = 0; i < bLength; i++)
        {
          data_buf[i] = static_cast<quint8>(data.mid(9 + i*2, 2).toInt(&ok, 16));
          if (!ok)
            {
              m_lastError = tr("File corrupt. Not hex value");
              file.close();
              return false;
            }
          cSum += data_buf[i];
        }

      cSum += data.mid(9 + bLength*2, 2).toInt(&ok, 16);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }

      if (cSum != 0)
        {
          m_lastError = tr("File corrupt. Checksum not correct");
          file.close();
          return false;
        }


      if(noteType == 0)  // binary data
        {
          quint32 address = lbaAddr + sbaAddr +offset;
          if ((address + static_cast<quint32>(bLength)) > MAX_CODE_SIZE)
            {
              m_lastError = tr("Address map very big");
              file.close();
              return false;
            }

          for (int i = 0; i < bLength; i++)
            m_flashBuf[address + static_cast<quint32>(i)] = data_buf[i];
          continue;
        }

      if(noteType == 1)  // end of file
        break;

      if(noteType == 2)  // address of segment
        {
          sbaAddr = static_cast<quint32>(data_buf[0] << 12) +
              static_cast<quint32>(data_buf[1] << 4);
          continue;
        }

      if(noteType == 4)  // linear address
        {
          lbaAddr = static_cast<quint32>(data_buf[0] << 24) +
              static_cast<quint32>(data_buf[1] << 16);
          continue;
        }
    }

  file.close();

  for (int i = sizeof(m_flashBuf) - 1; i>=0; i--)
    if (m_flashBuf[i] != 0xff)
      {
        m_flashLength = (static_cast<quint32>(i) + 0x100) & 0xffffff00;
        break;
      }

  return true;
}


bool UartLoaderWorker::loadHexLoader(QString path)
{
  QFile file(path);
  bool ok;
  quint32 sbaAddr = 0;  // address of segment
  quint32 lbaAddr = 0;  // extended address

  for(uint i = 0; i < sizeof(m_ramBuf); i++)
    {
      m_ramBuf[i] = 0xff;
    }

  if (!file.open(QIODevice::ReadOnly))
    {
      m_lastError = tr("Loader file not opened");
      return false;
    }

  while(1)
    {
      int bLength = 0;  // length of note
      quint16 offset = 0;  // address of data
      int noteType = 0;  // type of note
      quint8 cSum = 0;  // checksum
      quint8 data_buf[255];  // buffer of reading data

      if (file.atEnd())
        {
          break;
        }

      QByteArray data;
      data = file.readLine();
      if (data.isEmpty())
        continue;

      // parse line
      if (data.at(0) != ':')
        continue;
      bLength = data.mid(1, 2).toInt(&ok, 16);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }
      offset = static_cast<quint16>(data.mid(3, 2).toUInt(&ok, 16) << 8);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }
      offset += data.mid(5, 2).toInt(&ok, 16);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }
      noteType = data.mid(7, 2).toInt(&ok, 16);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }

      cSum = static_cast<quint8>(bLength + (offset >> 8) + (offset & 0xff) + noteType);

      if (bLength > (data.size() - 13))
        {
          m_lastError = tr("File corrupt. Line so short");
          file.close();
          return false;
        }

      for (int i = 0; i < bLength; i++)
        {
          data_buf[i] = static_cast<quint8>(data.mid(9 + i*2, 2).toUInt(&ok, 16));
          if (!ok)
            {
              m_lastError = tr("File corrupt. Not hex value");
              file.close();
              return false;
            }
          cSum += data_buf[i];
        }

      cSum += data.mid(9 + bLength*2, 2).toInt(&ok, 16);
      if (!ok)
        {
          m_lastError = tr("File corrupt. Not hex value");
          file.close();
          return false;
        }

      if (cSum != 0)
        {
          m_lastError = tr("File corrupt. Checksum not correct");
          file.close();
          return false;
        }


      if(noteType == 0)  // binary data
        {
          quint32 address = lbaAddr + sbaAddr + offset;
          quint32 ramBufAddress = address - m_ramStartAddress;
          if ((ramBufAddress + static_cast<quint32>(bLength)) > MAX_RAM_SIZE)
            {
              m_lastError = tr("Loader address not in ram");
              file.close();
              return false;
            }

          for (int i = 0; i < bLength; i++)
            m_ramBuf[ramBufAddress + static_cast<quint32>(i)] = data_buf[i];
          continue;
        }

      if(noteType == 1)  // end of file
        break;

      if(noteType == 2)  // address of segment
        {
          sbaAddr = static_cast<quint32>(data_buf[0] << 12) +
              static_cast<quint32>(data_buf[1] << 4);
          continue;
        }

      if(noteType == 4)  // linear address
        {
          lbaAddr = static_cast<quint32>(data_buf[0] << 24)  +
              static_cast<quint32>(data_buf[1] << 16);
          continue;
        }
    }

  file.close();

  // calculate start address
  for (uint i = 0; i < sizeof(m_ramBuf); i++)
    {
      if (m_ramBuf[i] != 0xff)
        {
          m_ramStartOffset = i;
          break;
        }
    }

  for (int i = sizeof(m_ramBuf) - 1; i >= 0; i--)
    if (m_ramBuf[i] != 0xff)
      {
        m_ramLength = (static_cast<quint32>(i) + 16 - m_ramStartOffset) & 0xfffffff8;
        break;
      }

  return true;
}

void UartLoaderWorker::identifyOfMode()
{
  QByteArray writeBuf;
  QByteArray readBuf;

  mode = None;

  if (m_portName.isEmpty() || m_baud <= 0)
    {
      emit signalWriteLog(tr("Port name is empty or baud not correct"));
      mode = PortBusy;
      return;
    }
  if (m_serialPort->isOpen())
    m_serialPort->close();
  m_serialPort->setPortName(m_portName);

  // check ram mode
  m_serialPort->setBaudRate(m_baud);
  if (!m_serialPort->open(QIODevice::ReadWrite))
    {
      qDebug() << "identifyOfMode - check ram mode: port not opened";
      emit signalWriteLog(tr("Port %1 not opened").arg(m_portName));
      mode = PortBusy;
      return;
    }

  writeBuf.append('I');
  m_serialPort->write(writeBuf);
  m_serialPort->flush();
  readBuf = readSerialData(12, 100);
  if ((readBuf.length() == 12) && readBuf.contains(m_idString))
    {
      mode = RamMode;
      qDebug() << "Identify RAM mode";
      m_serialPort->close();
      return;
    }

  // check raw mode and synchronization
  else
    {
      writeBuf.clear();
      writeBuf.append(static_cast<char>(0x0d));
      m_serialPort->write(writeBuf);
      m_serialPort->flush();
      readBuf = readSerialData(3, 100);
      if (readBuf.length() == 3 &&
          readBuf.at(0) == 0x0d &&
          readBuf.at(1) == 0x0a &&
          readBuf.at(2) == 0x3e)
        {
          mode = BareMode;
          qDebug() << "Identify RAW mode";
          m_serialPort->close();
          return;
        }

      // syncthing
      m_serialPort->close();
      m_serialPort->setBaudRate(QSerialPort::Baud9600);
      if (!m_serialPort->open(QIODevice::ReadWrite))
        {
          qDebug() << "identifyOfMode - synchronization: port not opened";
          emit signalWriteLog(tr("Port %1 not opened").arg(m_portName));
          mode = None;
          return;
        }
      writeBuf.clear();
      writeBuf.append(512, static_cast<char>(0x00));

      const int max_step = 2;
      for (int i = 0; i < max_step; i++)
        {
          m_serialPort->write(writeBuf);
          m_serialPort->flush();
          readBuf = readSerialData(3, 1000);
          if (readBuf.length() == 3 &&
              readBuf.at(0) == 0x0d &&
              readBuf.at(1) == 0x0a &&
              readBuf.at(2) == 0x3e)
            {
              break;
            }
          else if (i == (max_step - 1))
            {
              qDebug() << "Synchronization failed";
              m_serialPort->close();
              emit signalWriteLog(tr("Synchronization failed"));
              return;
            }
        }

      qDebug() << "Synchronization done";
      emit signalWriteLog(tr("Synchronization done"));

      if ((m_baud > 0) && (m_baud != 9600))
        {
          // set baud
          writeBuf.clear();
          writeBuf.append('B');
          writeBuf.append(static_cast<char>(m_baud & 0xff));
          writeBuf.append(static_cast<char>((m_baud >> 8) & 0xff));
          writeBuf.append(static_cast<char>((m_baud >> 16) & 0xff));
          writeBuf.append(static_cast<char>((m_baud >> 24) & 0xff));
          m_serialPort->write(writeBuf);
          m_serialPort->flush();

          readBuf = readSerialData(1, 1000);

          if (readBuf.length() < 1 ||
              readBuf.at(0) != 'B')
            {
              qDebug() << "identifyOfMode, set baud: transmit error";
              emit signalWriteLog(tr("Set baud failed"));
              m_serialPort->close();
              mode = None;
              return;
            }
        }

      m_serialPort->close();
      mode = BareMode;
      return;
    }
}

QByteArray UartLoaderWorker::readSerialData(int size, int timeout)
{
  QByteArray readBuf;
  while (m_serialPort->waitForReadyRead(timeout))
    {
      readBuf.append(m_serialPort->readAll());
      if (readBuf.size() == size)
        break;
    }

  return readBuf;
}
