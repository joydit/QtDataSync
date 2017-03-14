#ifndef REMOTECONNECTOR_H
#define REMOTECONNECTOR_H

#include "QtDataSync/qdatasync_global.h"
#include "QtDataSync/defaults.h"
#include "QtDataSync/stateholder.h"

#include <QtCore/qjsonvalue.h>
#include <QtCore/qjsonobject.h>
#include <QtCore/qobject.h>
#include <QtCore/qfutureinterface.h>

namespace QtDataSync {

class Authenticator;

class Q_DATASYNC_EXPORT RemoteConnector : public QObject
{
	Q_OBJECT

public:
	explicit RemoteConnector(QObject *parent = nullptr);

	virtual void initialize(Defaults *defaults);
	virtual void finalize();

	virtual Authenticator *createAuthenticator(Defaults *defaults, QObject *parent) = 0;

public Q_SLOTS:
	virtual void reloadRemoteState() = 0;

	virtual void download(const ObjectKey &key, const QByteArray &keyProperty) = 0;
	virtual void upload(const ObjectKey &key, const QJsonObject &object, const QByteArray &keyProperty) = 0;//auto unchanged
	virtual void remove(const ObjectKey &key, const QByteArray &keyProperty) = 0;//auto unchanged
	virtual void markUnchanged(const ObjectKey &key, const QByteArray &keyProperty) = 0;

	virtual void localResyncCompleted() = 0;//TODO call from engine

	void resetUserId(QFutureInterface<QVariant> futureInterface, const QVariant &extraData);

Q_SIGNALS:
	void remoteStateLoaded(bool canUpdate, const StateHolder::ChangeHash &remoteChanges);
	void remoteDataChanged(const ObjectKey &key, StateHolder::ChangeState state);
	void authenticationFailed(const QString &reason);
	void clearAuthenticationError();

	void operationDone(const QJsonValue &result = QJsonValue::Undefined);
	void operationFailed(const QString &error);

	void requestLocalResync();//TODO use in engine

protected:
	Defaults *defaults() const;

	virtual QByteArray loadDeviceId();
	virtual void resetUserData(const QVariant &extraData) = 0;

private:
	Defaults *_defaults;//TODO d ptr
};

}

#endif // REMOTECONNECTOR_H
