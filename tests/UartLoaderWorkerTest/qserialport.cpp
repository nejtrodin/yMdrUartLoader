#include "qserialport.h"
#include <QtTest>

#include "mcumemorysingleton.h"

QSerialPort::QSerialPort(QObject *parent) :
    QObject(parent)
{
    m_portOpened = false;
    m_mcuMode = BareWaitMode;
}

void QSerialPort::setPortName(QString portName)
{
    Q_UNUSED(portName)
}

bool QSerialPort::setBaudRate(qint32 baudRate)
{
    Q_UNUSED(baudRate)
    return true;
}

bool QSerialPort::open(QIODevice::OpenMode mode)
{
    Q_UNUSED(mode)
    m_portOpened = true;
    return true;
}

void QSerialPort::close()
{
    m_portOpened = false;
}

QString QSerialPort::errorString() const
{
    return QString("Error string");
}

bool QSerialPort::flush()
{
    return true;
}

qint64 QSerialPort::write(const QByteArray &data)
{       
    if (m_mcuMode == BareWaitMode)
    {
        // sync
        if (data.size() >= 1 && data.at(0) == static_cast<char>(0x0d))
        {
            m_answerData.append(static_cast<char>(0x0d));
            m_answerData.append(static_cast<char>(0x0a));
            m_answerData.append(static_cast<char>(0x3e));
        }

        // get revision command
        else if (data.size() >= 9 &&
                 data.at(0) == 'Y' &&
                 data.at(1) == static_cast<char>(0xff) &&
                 data.at(2) == static_cast<char>(0x0f) &&
                 data.at(3) == static_cast<char>(0x00) &&
                 data.at(4) == static_cast<char>(0x00) &&
                 data.at(5) == static_cast<char>(0x01) &&
                 data.at(6) == static_cast<char>(0x00) &&
                 data.at(7) == static_cast<char>(0x00) &&
                 data.at(8) == static_cast<char>(0x00))
        {
            m_answerData.append('Y');
            m_answerData.append(static_cast<char>(0xcd));  // revision 4
            m_answerData.append('K');
        }

        // load command
        else if (data.size() >= 9 && data.at(0) == 'L')
        {
            m_answerData.append('L');
            m_mcuMode = BareLoadMode;
            m_inputCache = data;
        }

        // verify command
        else if (data.size() >= 9 && data.at(0) == 'Y')
        {
            m_answerData.append('Y');
            quint32 address = static_cast<quint32>(data.at(1)) & 0xff;
            address |= (static_cast<quint32>(data.at(2)) & 0xff) << 8;
            address |= (static_cast<quint32>(data.at(3)) & 0xff) << 16;
            address |= (static_cast<quint32>(data.at(4)) & 0xff) << 24;
            int length = static_cast<int>(data.at(5)) +
                    (static_cast<int>(data.at(6)) << 8) +
                    (static_cast<int>(data.at(7)) << 16) +
                    (static_cast<int>(data.at(8)) << 24);
            McuMemorySingleton* mcuMemory = McuMemorySingleton::instance();
            QByteArray readData = mcuMemory->read(address, length);
            m_answerData.append(readData);
            m_answerData.append('K');
        }

        // run address command
        else if (data.size() >= 5 && data.at(0) == 'R')
        {
            m_answerData.append('R');
            m_mcuMode = RamMode;
        }
    }

    else if (m_mcuMode == BareLoadMode)
    {
        m_inputCache.append(data);

        if (m_inputCache.size() >= 9 && m_inputCache.at(0) == 'L')
        {
            quint32 address = static_cast<quint32>(m_inputCache.at(1)) & 0xff;
            address |= (static_cast<quint32>(m_inputCache.at(2)) & 0xff) << 8;
            address |= (static_cast<quint32>(m_inputCache.at(3)) & 0xff) << 16;
            address |= (static_cast<quint32>(m_inputCache.at(4)) & 0xff) << 24;
            int length = static_cast<int>(m_inputCache.at(5)) +
                   (static_cast<int>(m_inputCache.at(6)) << 8) +
                   (static_cast<int>(m_inputCache.at(7)) << 16) +
                   (static_cast<int>(m_inputCache.at(8)) << 24);

           if (m_inputCache.size() >= (9 + length))
           {
               McuMemorySingleton* mcuMemory = McuMemorySingleton::instance();
               QByteArray binData = m_inputCache.mid(9, length);
               mcuMemory->write(address, binData);

               m_answerData.append('K');
               m_mcuMode = BareWaitMode;
               m_inputCache.clear();
           }
        }
    }

    else if (m_mcuMode == RamMode)
    {
        static quint32 currentAddress = 0;

        if (data.size() >= 1 && data.at(0) == 'I')
            m_answerData.append("1986BOOTUART");

        else if (data.size() >= 1 && data.at(0) == 'E')
        {
            m_answerData.append('E');
            // address
            m_answerData.append(static_cast<char>(0x00));
            m_answerData.append(static_cast<char>(0x00));
            m_answerData.append(static_cast<char>(0x02));
            m_answerData.append(static_cast<char>(0x00));
            // data
            m_answerData.append(static_cast<char>(0xff));
            m_answerData.append(static_cast<char>(0xff));
            m_answerData.append(static_cast<char>(0xff));
            m_answerData.append(static_cast<char>(0xff));
        }

        else if (data.size() >= 5 && data.at(0) == 'A')
        {
            currentAddress = (static_cast<quint32>(data.at(1)) & 0xff) |
                    ((static_cast<quint32>(data.at(2)) & 0xff) << 8) |
                    ((static_cast<quint32>(data.at(3)) & 0xff) << 16) |
                    ((static_cast<quint32>(data.at(4)) & 0xff) << 24);
            m_answerData.append(static_cast<char>(0x00));
        }

        else if (data.size() >= 257 && data.at(0) == 'P')
        {
            QByteArray writeData = data.mid(1, 256);
            McuMemorySingleton* mcuMemory = McuMemorySingleton::instance();
            mcuMemory->write(currentAddress, writeData);
            currentAddress += 256;

            QByteArray::iterator it;
            quint8 csum = 0;

            for (it = writeData.begin(); it != writeData.end(); ++it)
                csum += static_cast<quint8>(*it);
            m_answerData.append(static_cast<char>(csum));
        }

        else if (data.size() >= 1 && data.at(0) == 'V')
        {
            McuMemorySingleton* mcuMemory = McuMemorySingleton::instance();
            m_answerData.append(mcuMemory->read(currentAddress, 8));
            currentAddress += 8;
        }

        else if (data.size() >= 1 && data.at(0) == 'R')
            m_answerData.append('R');
    }

    return data.size();
}

bool QSerialPort::waitForReadyRead(int msecs)
{
    Q_UNUSED(msecs)

    if (m_answerData.isEmpty())
        return false;
    else
        return true;
}

QByteArray QSerialPort::readAll()
{
    QByteArray result = m_answerData;
    m_answerData.clear();
    return result;
}
