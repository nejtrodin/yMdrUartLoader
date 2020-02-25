#ifndef MCUMEMORYSINGLETON_H
#define MCUMEMORYSINGLETON_H

#include <QObject>


/*!
 * \brief The McuMemorySingleton class provides access to the memory of MCU.
 */
class McuMemorySingleton
{
private:
    McuMemorySingleton() {}

public:
    static McuMemorySingleton* instance() {
        static McuMemorySingleton* _instance = nullptr;
        if (_instance == nullptr) {
            _instance = new McuMemorySingleton();
        }
        return _instance;
    }

    bool write(quint32 address, QByteArray data);
    QByteArray read(quint32 address, int length);

private:
    static const int ram1Size = 32*1024;
    static const int ram1StartAddress = 0x20000000;
    static const int ram2Size = 16*1024;
    static const int ram2StartAddress = 0x20100000;
    static const int flash1Size = 128*1024;
    static const int flash1StartAddress = 0x00000000;

    quint8 m_ram1Buf[ram1Size];
    quint8 m_ram2Buf[ram2Size];
    quint8 m_flash1Buf[flash1Size];

};

#endif // MCUMEMORYSINGLETON_H
