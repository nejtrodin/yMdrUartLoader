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
    void writeLoaderTest();
    void eraseFlashTest();
    void writeFlashTest();
    void verifyFlashTest();
    void runMcuTest();
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

    worker.slotSetPortSettings("/dev/ttyUSB0", 9600);
    worker.slotGetRevision();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
    QCOMPARE(arguments.at(1).toString(), "Revision: 4");
}

void UartLoaderWorkerTest::writeLoaderTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalLoaderWrited(bool)));

    worker.slotSetPortSettings("/dev/ttyUSB0", 9600);
    worker.slotWriteLoader();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);

    QFile templateFile("://testFiles/1986ve1bootuart.bin");
    templateFile.open(QFile::ReadOnly);
    QByteArray templateData= templateFile.readAll();

    McuMemorySingleton* mcuMemory = McuMemorySingleton::instance();
    QByteArray readData = mcuMemory->read(0x20103400, templateData.size());

    QCOMPARE(readData, templateData);
}

void UartLoaderWorkerTest::eraseFlashTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalFlashErased(bool)));

    worker.slotSetPortSettings("/dev/ttyUSB0", 9600);
    worker.slotEraseFlash();
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

void UartLoaderWorkerTest::writeFlashTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalFlashWrited(bool)));

    worker.slotSetPortSettings("/dev/ttyUSB0", 9600);
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

void UartLoaderWorkerTest::verifyFlashTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalFlashVerified(bool)));

    worker.slotSetPortSettings("/dev/ttyUSB0", 9600);
    worker.slotWriteFlash("://testFiles/1986ve1t_test.hex");
    worker.slotVerifyFlash("://testFiles/1986ve1t_test.hex");

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

void UartLoaderWorkerTest::runMcuTest()
{
    UartLoaderWorker worker;
    QSignalSpy spy(&worker, SIGNAL(signalMcuRuned(bool)));

    worker.slotSetPortSettings("/dev/ttyUSB0", 9600);
    worker.slotRunMcu();

    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toBool(), true);
}

QTEST_APPLESS_MAIN(UartLoaderWorkerTest)

#include "tst_uartloaderworkertest.moc"
