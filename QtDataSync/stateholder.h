#ifndef STATEHOLDER_H
#define STATEHOLDER_H

#include "qtdatasync_global.h"
#include <QHash>
#include <QObject>

namespace QtDataSync {

class QTDATASYNCSHARED_EXPORT StateHolder : public QObject
{
	Q_OBJECT

public:
	enum ChangeState {
		Unchanged = 0,
		Changed = 1,
		Deleted = 2
	};
	Q_ENUM(ChangeState)

	struct ChangedInfo {
		QByteArray typeName;
		QString key;
		ChangeState state;
	};

	explicit StateHolder(QObject *parent = nullptr);

	virtual void initialize();
	virtual void finalize();

	virtual QList<ChangedInfo> listLocalChanges() = 0;
	virtual void markLocalChanged(const QByteArray &typeName, const QString &key, ChangeState changed) = 0;
	virtual void markAllLocalChanged(const QByteArray &typeName, ChangeState changed) = 0;
};

}

#endif // STATEHOLDER_H
