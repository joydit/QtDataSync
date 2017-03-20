#include <QString>
#include <QtTest>
#include <QCoreApplication>
#include "tst.h"
#include "QtDataSync/private/wsremoteconnector_p.h"

using namespace QtDataSync;

class WsRemoteConnectorTest : public QObject
{
	Q_OBJECT

private Q_SLOTS:
	void initTestCase();
	void cleanupTestCase();

	void testServerConnecting();
	void testUpload_data();
	void testUpload();
	void testReloadAndResync();
	//void testDownload();
	//void testRemove();
	//void testMarkUnchanged();

	//void testSecondDevice();

Q_SIGNALS:
	void unlockSignal();

private:
	AsyncDataStore *async;
	SyncController *controller;
	WsAuthenticator *auth;

	MockLocalStore *store;
	MockStateHolder *holder;
};

void WsRemoteConnectorTest::initTestCase()
{
#ifdef Q_OS_UNIX
	Q_ASSERT(qgetenv("LD_PRELOAD").contains("Qt5DataSync"));
#endif

	tst_init();

	//create setup to "init" both of them, but datasync itself is not used here
	Setup setup;
	mockSetup(setup);
	store = static_cast<MockLocalStore*>(setup.localStore());
	holder = static_cast<MockStateHolder*>(setup.stateHolder());
	setup.setRemoteConnector(new WsRemoteConnector())
			.create();

	async = new AsyncDataStore(this);
	controller = new SyncController(this);
	auth = Setup::authenticatorForSetup<WsAuthenticator>(this);
}

void WsRemoteConnectorTest::cleanupTestCase()
{
	delete auth;
	delete controller;
	delete async;
	Setup::removeSetup(Setup::DefaultSetup);
}

void WsRemoteConnectorTest::testServerConnecting()
{
	QSignalSpy syncSpy(controller, &SyncController::syncStateChanged);
	QSignalSpy conSpy(auth, &WsAuthenticator::connectedChanged);

	QVERIFY(syncSpy.wait());
	QCOMPARE(controller->syncState(), SyncController::Disconnected);
	QVERIFY(!auth->isConnected());

	//empty store -> used INVALID id
	auth->setRemoteUrl(QStringLiteral("ws://localhost:4242"));
	auth->setServerSecret("baum42");
	auth->setUserIdentity("invalid");
	auth->reconnect();

	QVERIFY(conSpy.wait());
	QVERIFY(!auth->isConnected());
	for(auto i = 0; i < 10 && syncSpy.count() < 1; i++)
		syncSpy.wait(500);
	QCOMPARE(syncSpy.count(), 1);
	QCOMPARE(syncSpy[0][0], QVariant::fromValue(SyncController::Disconnected));
	QCOMPARE(controller->syncState(), SyncController::Disconnected);

	//now connect again, but reset id first
	conSpy.clear();
	syncSpy.clear();
	auth->resetUserIdentity();
	auth->reconnect();

	QVERIFY(conSpy.wait());
	QVERIFY(auth->isConnected());
	for(auto i = 0; i < 10 && syncSpy.count() < 3; i++)
		syncSpy.wait(500);
	QCOMPARE(syncSpy.count(), 3);
	QCOMPARE(syncSpy[0][0], QVariant::fromValue(SyncController::Loading));
	QCOMPARE(syncSpy[1][0], QVariant::fromValue(SyncController::Syncing));
	QCOMPARE(syncSpy[2][0], QVariant::fromValue(SyncController::Synced));
	QCOMPARE(controller->syncState(), SyncController::Synced);

	//now reconnect, to test if "login" works as well
	conSpy.clear();
	syncSpy.clear();
	auth->reconnect();

	QVERIFY(conSpy.wait());
	QVERIFY(!auth->isConnected());
	QVERIFY(conSpy.wait());
	QVERIFY(auth->isConnected());
	for(auto i = 0; i < 10 && syncSpy.count() < 4; i++)
		syncSpy.wait(500);
	QCOMPARE(syncSpy.count(), 4);
	QCOMPARE(syncSpy[0][0], QVariant::fromValue(SyncController::Disconnected));
	QCOMPARE(syncSpy[1][0], QVariant::fromValue(SyncController::Loading));
	QCOMPARE(syncSpy[2][0], QVariant::fromValue(SyncController::Syncing));
	QCOMPARE(syncSpy[3][0], QVariant::fromValue(SyncController::Synced));
	QCOMPARE(controller->syncState(), SyncController::Synced);
}

void WsRemoteConnectorTest::testUpload_data()
{
	QTest::addColumn<TestData>("data");

	QTest::newRow("data0") << generateData(310);
	QTest::newRow("data1") << generateData(311);
	QTest::newRow("data2") << generateData(312);
	QTest::newRow("data3") << generateData(313);
	QTest::newRow("data4") << generateData(314);
	QTest::newRow("data5") << generateData(315);
	QTest::newRow("data6") << generateData(316);
	QTest::newRow("data7") << generateData(317);
}

void WsRemoteConnectorTest::testUpload()
{
	QFETCH(TestData, data);

	try {
		async->save<TestData>(data).waitForFinished();
		QThread::msleep(200);
		QVERIFY(async->keys<TestData>().result().isEmpty());//make shure local store is disabled
	} catch(QException &e) {
		QFAIL(e.what());
	}
}

void WsRemoteConnectorTest::testReloadAndResync()
{
	//IMPORTANT!!! clear pending notifies...
	QCoreApplication::processEvents();

	QSignalSpy unlockSpy(this, &WsRemoteConnectorTest::unlockSignal);

	store->mutex.lock();
	holder->mutex.lock();
	store->enabled = true;
	holder->enabled = true;
	holder->mutex.unlock();
	store->mutex.unlock();

	controller->triggerSyncWithResult([&](SyncController::SyncState state){
		QCOMPARE(state, SyncController::Synced);
		emit unlockSignal();
	});

	QVERIFY(unlockSpy.wait());
	try {
		//sync wont help, no obvious changes
		QVERIFY(async->keys<TestData>().result().isEmpty());
	} catch(QException &e) {
		QFAIL(e.what());
	}

	controller->triggerResyncWithResult([&](SyncController::SyncState state){
		QCOMPARE(state, SyncController::Synced);
		emit unlockSignal();
	});

	QVERIFY(unlockSpy.wait());
	try {
		//resync however should load all
		QCOMPARE(async->count<TestData>().result(), 8);
	} catch(QException &e) {
		QFAIL(e.what());
	}
}

QTEST_MAIN(WsRemoteConnectorTest)

#include "tst_wsremoteconnector.moc"