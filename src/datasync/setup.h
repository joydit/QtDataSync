#ifndef QTDATASYNC_SETUP_H
#define QTDATASYNC_SETUP_H

#include "QtDataSync/qtdatasync_global.h"
#include "QtDataSync/defaults.h"

#include <QtCore/qobject.h>

#include <functional>

class QJsonSerializer;

namespace QtDataSync {

class SetupPrivate;
//! The class to setup and create datasync instances
class Q_DATASYNC_EXPORT Setup
{
	Q_DISABLE_COPY(Setup)

public:
	//! The default setup name
	static const QString DefaultSetup;

	//! Sets the maximum timeout for shutting down setups
	static void setCleanupTimeout(unsigned long timeout);
	//! Stops the datasync instance and removes it
	static void removeSetup(const QString &name, bool waitForFinished = false);

	//! Constructor
	Setup();
	//! Destructor
	~Setup();

	//! Returns the setups local directory
	QString localDir() const;
	//! Returns the setups json serializer
	QJsonSerializer *serializer() const;
	//! Returns the additional property with the given key
	QVariant property(const QByteArray &key) const;
	//! Returns the fatal error handler to be used by the Logger
	std::function<void(QString,bool,QString)> fatalErrorHandler() const;

	//! Sets the setups local directory
	Setup &setLocalDir(QString localDir);
	//! Sets the setups json serializer
	Setup &setSerializer(QJsonSerializer *serializer);
	//! Sets the additional property with the given key to data
	Setup &setProperty(const QByteArray &key, const QVariant &data);
	//! Sets the fatal error handler to be used by the Logger
	Setup &setFatalErrorHandler(const std::function<void(QString,bool,QString)> &fatalErrorHandler);

	//! Creates a datasync instance from this setup with the given name
	void create(const QString &name = DefaultSetup);

private:
	QScopedPointer<SetupPrivate> d;
};

}

#endif // QTDATASYNC_SETUP_H
