#ifndef QSERIALPORT_H
#define QSERIALPORT_H

#include <QObject>
#include <QIODevice>

/*!
 * \brief The QSerialPort class emulates QSerialPort class of Qt library
 */
class QSerialPort : public QObject
{
    Q_OBJECT
public:
    enum BaudRate {
        Baud1200 = 1200,
        Baud2400 = 2400,
        Baud4800 = 4800,
        Baud9600 = 9600,
        Baud19200 = 19200,
        Baud38400 = 38400,
        Baud57600 = 57600,
        Baud115200 = 115200,
        UnknownBaud = -1
    };
    Q_ENUM(BaudRate)

    QSerialPort(QObject *parent = nullptr);
    void setPortName(QString portName);
    bool setBaudRate(qint32 baudRate);
    bool open(QIODevice::OpenMode mode);
    void close();
    bool isOpen() const {return m_portOpened;}
    QString errorString() const;
    bool flush();
    qint64 write(const QByteArray &data);
    bool waitForReadyRead(int msecs = 30000);
    QByteArray readAll();


private:
    enum McuMode {
        BareWaitMode,
        BareLoadMode,
        RamMode
    };

    bool m_portOpened;
    McuMode m_mcuMode;
    QByteArray m_answerData;
    QByteArray m_inputCache;

    enum {
      Type1986VE1T,
      Type1986VE9x
    } m_mcuType;

};

#endif // QSERIALPORT_H
