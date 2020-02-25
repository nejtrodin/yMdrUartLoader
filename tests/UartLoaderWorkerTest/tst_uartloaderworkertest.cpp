#include <QtTest>
#include <QSignalSpy>

// add necessary includes here
#include "uartloaderworker.h"
#include "mcumemorysingleton.h"

class UartLoaderWorkerTest : public QObject
{
    Q_OBJECT

public:
    UartLoaderWorkerTest();
    ~UartLoaderWorkerTest();

private slots:
    void getRevisionTest();
    void writeLoaderVe1tTest();
    void writeLoaderVe9xTest();
    void eraseFlashVe1tTest();
    void eraseFlashVe9xTest();
    void writeFlashVe1tTest();
    void writeFlashVe9xTest();
    void verifyFlashVe1tTest();
    void verifyFlashVe9xTest();
    void runMcuVe1tTest();
    void runMcuVe9xTest();
};

UartLoaderWorkerTest::UartLoaderWorkerTest()
{

}

UartLoaderWorkerTest::~UartLoaderWorkerTest()
{

}

void UartLoaderWorkerTest::getRevisionTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalRevision(bool, QString)));

    // test 1986ve1t
    worker.slotSetSettings("1986ВЕ1Т", "/dev/ttyUSB0", 9600);
    worker.slotGetRevision();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
    QCOMPARE(arguments.at(1).toString(), "Revision: 4");


    // test 1986ve9x
    worker.slotSetSettings("1986ВЕ9X", "/dev/ttyUSB0", 9600);
    worker.slotGetRevision();

    QCOMPARE(spy.count(), 1);
    arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
    QCOMPARE(arguments.at(1).toString(), "Revision: 4");
}

void UartLoaderWorkerTest::writeLoaderVe1tTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalLoaderWrited(bool)));

    worker.slotSetSettings("1986ВЕ1Т", "/dev/ttyUSB0", 9600);
    worker.slotWriteLoader();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);

    QFile templateFile("://testFiles/1986ve1bootuart.bin");
    templateFile.open(QFile::ReadOnly);
    QByteArray templateData= templateFile.readAll();
    templateFile.close();

    McuMemorySingleton* mcuMemory = McuMemorySingleton::instance();
    QByteArray readData = mcuMemory->read(0x20103400, templateData.size());

    QCOMPARE(readData, templateData);
}

void UartLoaderWorkerTest::writeLoaderVe9xTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalLoaderWrited(bool)));

    worker.slotSetSettings("1986ВЕ9X", "/dev/ttyUSB0", 9600);
    worker.slotWriteLoader();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);

    QFile templateFile("://testFiles/1986_BOOT_UART.bin");
    templateFile.open(QFile::ReadOnly);
    QByteArray templateData= templateFile.readAll();
    templateFile.close();

    McuMemorySingleton* mcuMemory = McuMemorySingleton::instance();
    QByteArray readData = mcuMemory->read(0x20007800, templateData.size());

    QCOMPARE(readData, templateData);
}

void UartLoaderWorkerTest::eraseFlashVe1tTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalFlashErased(bool)));

    worker.slotSetSettings("1986ВЕ1Т", "/dev/ttyUSB0", 9600);
    worker.slotEraseFlash();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

void UartLoaderWorkerTest::eraseFlashVe9xTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalFlashErased(bool)));

    worker.slotSetSettings("1986ВЕ9X", "/dev/ttyUSB0", 9600);
    worker.slotEraseFlash();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

void UartLoaderWorkerTest::writeFlashVe1tTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalFlashWrited(bool)));

    worker.slotSetSettings("1986ВЕ1Т", "/dev/ttyUSB0", 9600);
    worker.slotWriteFlash("://testFiles/1986ve1t_test.hex");
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);

    QFile templateFile("://testFiles/1986ve1t_test.bin");
    templateFile.open(QFile::ReadOnly);
    QByteArray templateData= templateFile.readAll();
    templateFile.close();

    McuMemorySingleton* mcuMemory = McuMemorySingleton::instance();
    QByteArray readData = mcuMemory->read(0x00, templateData.size());

    QCOMPARE(readData, templateData);
}

void UartLoaderWorkerTest::writeFlashVe9xTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalFlashWrited(bool)));

    worker.slotSetSettings("1986ВЕ9X", "/dev/ttyUSB0", 9600);
    worker.slotWriteFlash("://testFiles/1986ve92_test.hex");
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);

    QFile templateFile("://testFiles/1986ve92_test.bin");
    templateFile.open(QFile::ReadOnly);
    QByteArray templateData= templateFile.readAll();
    templateFile.close();

    McuMemorySingleton* mcuMemory = McuMemorySingleton::instance();
    QByteArray readData = mcuMemory->read(0x08000000, templateData.size());

    QFile rFile("readData.bin");
    rFile.open(QFile::WriteOnly);
    rFile.write(readData);
    rFile.close();


    QCOMPARE(readData, templateData);
}

void UartLoaderWorkerTest::verifyFlashVe1tTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalFlashVerified(bool)));

    worker.slotSetSettings("1986ВЕ1Т", "/dev/ttyUSB0", 9600);
    worker.slotWriteFlash("://testFiles/1986ve1t_test.hex");
    worker.slotVerifyFlash("://testFiles/1986ve1t_test.hex");

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

void UartLoaderWorkerTest::verifyFlashVe9xTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalFlashVerified(bool)));

    worker.slotSetSettings("1986ВЕ9X", "/dev/ttyUSB0", 9600);
    worker.slotWriteFlash("://testFiles/1986ve92_test.hex");
    worker.slotVerifyFlash("://testFiles/1986ve92_test.hex");

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

void UartLoaderWorkerTest::runMcuVe1tTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalMcuRuned(bool)));

    worker.slotSetSettings("1986ВЕ1Т", "/dev/ttyUSB0", 9600);
    worker.slotRunMcu();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

void UartLoaderWorkerTest::runMcuVe9xTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalMcuRuned(bool)));

    worker.slotSetSettings("1986ВЕ9X", "/dev/ttyUSB0", 9600);
    worker.slotRunMcu();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

QTEST_APPLESS_MAIN(UartLoaderWorkerTest)

#include "tst_uartloaderworkertest.moc"
