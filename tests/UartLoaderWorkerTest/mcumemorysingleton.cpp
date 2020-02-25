#include "mcumemorysingleton.h"

#include "QDebug"


bool McuMemorySingleton::write(quint32 address, QByteArray data)
{
    bool result = false;

    if (address >= ram2StartAddress && address < ram2StartAddress + ram2Size)
    {
        int bufIdx = static_cast<int>(address) - ram2StartAddress;
        int lastIdx = bufIdx + data.size();
        if (lastIdx >= ram2Size)
            lastIdx = ram2Size - 1;
        QByteArray::iterator dataIt = data.begin();
        for (; bufIdx < lastIdx; bufIdx++)
            m_ram2Buf[bufIdx] = static_cast<quint8>(*dataIt++);

        result = true;
    }
    else if (address >= flash1StartAddress &&
             address < flash1StartAddress + flash1Size)
    {
        int bufIdx = static_cast<int>(address) - flash1StartAddress;
        int lastIdx = bufIdx + data.size();
        if (lastIdx >= flash1Size)
            lastIdx = flash1Size - 1;
        QByteArray::iterator dataIt = data.begin();
        for (; bufIdx < lastIdx; bufIdx++)
            m_flash1Buf[bufIdx] = static_cast<quint8>(*dataIt++);

        result = true;
    }

    else
        qDebug() << "Write memory. Address is not in map: " << address;

    return result;
}

QByteArray McuMemorySingleton::read(quint32 address, int length)
{
    QByteArray result;

    if (address >= ram2StartAddress && address < ram2StartAddress + ram2Size)
    {
        int bufIdx = static_cast<int>(address) - ram2StartAddress;
        int lastIdx = bufIdx + length;
        if (lastIdx >= ram2Size)
            lastIdx = ram2Size - 1;
        for (; bufIdx < lastIdx; bufIdx++)
            result.append(static_cast<char>(m_ram2Buf[bufIdx]));
    }
    else if (address >= flash1StartAddress &&
             address < flash1StartAddress + flash1Size)
    {
        int bufIdx = static_cast<int>(address) - flash1StartAddress;
        int lastIdx = bufIdx + length;
        if (lastIdx >= flash1Size)
            lastIdx = flash1Size - 1;
        for (; bufIdx < lastIdx; bufIdx++)
            result.append(static_cast<char>(m_flash1Buf[bufIdx]));
    }
    else
        qDebug() << "Read memory. Address is not in map: " << address;

    return result;
}
