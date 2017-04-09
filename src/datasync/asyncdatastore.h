#ifndef QTDATASYNC_ASYNCDATASTORE_H
#define QTDATASYNC_ASYNCDATASTORE_H

#include "QtDataSync/qtdatasync_global.h"
#include "QtDataSync/task.h"

#include <QtCore/qobject.h>
#include <QtCore/qfuture.h>
#include <QtCore/qmetaobject.h>

namespace QtDataSync {

class Setup;

class AsyncDataStorePrivate;
//! A class to access data asynchronously
class Q_DATASYNC_EXPORT AsyncDataStore : public QObject
{
	Q_OBJECT

public:
	//! Constructs a store for the default setup
	explicit AsyncDataStore(QObject *parent = nullptr);
	//! Constructs a store for the given setup
	explicit AsyncDataStore(const QString &setupName, QObject *parent = nullptr);
	//! Destructor
	~AsyncDataStore();

	//! @copybrief AsyncDataStore::count()
	GenericTask<int> count(int metaTypeId);
	//! @copybrief AsyncDataStore::keys()
	GenericTask<QStringList> keys(int metaTypeId);
	//! @copybrief AsyncDataStore::loadAll()
	Task loadAll(int dataMetaTypeId, int listMetaTypeId);
	//! @copybrief AsyncDataStore::load(const QString &)
	Task load(int metaTypeId, const QString &key);
	//! @copybrief AsyncDataStore::load(const K &)
	Task load(int metaTypeId, const QVariant &key);
	//! @copybrief AsyncDataStore::save(const T &)
	Task save(int metaTypeId, const QVariant &value);
	//! @copybrief AsyncDataStore::remove(const QString &)
	Task remove(int metaTypeId, const QString &key);
	//! @copybrief AsyncDataStore::remove(const K &)
	Task remove(int metaTypeId, const QVariant &key);
	//! @copybrief AsyncDataStore::search(const QString &)
	Task search(int dataMetaTypeId, int listMetaTypeId, const QString &query);

	//! Counts the number of datasets for the given type
	template<typename T>
	GenericTask<int> count();
	//! Returns all saved keys for the given type
	template<typename T>
	GenericTask<QStringList> keys();
	//! Loads all existing datasets for the given type
	template<typename T>
	GenericTask<QList<T>> loadAll();
	//! Loads the dataset with the given key for the given type
	template<typename T>
	GenericTask<T> load(const QString &key);
	//! @copybrief AsyncDataStore::load(const QString &)
	template<typename T, typename K>
	GenericTask<T> load(const K &key);
	//! Saves the given dataset in the store
	template<typename T>
	GenericTask<void> save(const T &value);
	//! Removes the dataset with the given key for the given type
	template<typename T>
	GenericTask<bool> remove(const QString &key);
	//! @copybrief AsyncDataStore::remove(const QString &)
	template<typename T, typename K>
	GenericTask<bool> remove(const K &key);
	//! Searches the store for datasets of the given type where the key matches the query
	template<typename T>
	GenericTask<QList<T>> search(const QString &query);

Q_SIGNALS:
	//! Will be emitted when a dataset in the store has changed
	void dataChanged(int metaTypeId, const QString &key, bool wasDeleted);
	//! Will be emitted when the store has be reset (cleared)
	void dataResetted();

private:
	QScopedPointer<AsyncDataStorePrivate> d;

	QFutureInterface<QVariant> internalCount(int metaTypeId);
	QFutureInterface<QVariant> internalKeys(int metaTypeId);
	QFutureInterface<QVariant> internalLoadAll(int dataMetaTypeId, int listMetaTypeId);
	QFutureInterface<QVariant> internalLoad(int metaTypeId, const QString &key);
	QFutureInterface<QVariant> internalSave(int metaTypeId, const QVariant &value);
	QFutureInterface<QVariant> internalRemove(int metaTypeId, const QString &key);
	QFutureInterface<QVariant> internalSearch(int dataMetaTypeId, int listMetaTypeId, const QString &query);
};

template<typename T>
GenericTask<int> AsyncDataStore::count()
{
	return internalCount(qMetaTypeId<T>());
}

template<typename T>
GenericTask<QStringList> AsyncDataStore::keys()
{
	return internalKeys(qMetaTypeId<T>());
}

template<typename T>
GenericTask<QList<T>> AsyncDataStore::loadAll()
{
	return internalLoadAll(qMetaTypeId<T>(), qMetaTypeId<QList<T>>());
}

template<typename T>
GenericTask<T> AsyncDataStore::load(const QString &key)
{
	return internalLoad(qMetaTypeId<T>(), key);
}

template<typename T, typename K>
GenericTask<T> AsyncDataStore::load(const K &key)
{
	return internalLoad(qMetaTypeId<T>(), QVariant::fromValue(key).toString());
}

template<typename T>
GenericTask<void> AsyncDataStore::save(const T &value)
{
	return internalSave(qMetaTypeId<T>(), QVariant::fromValue(value));
}

template<typename T>
GenericTask<bool> AsyncDataStore::remove(const QString &key)
{
	return internalRemove(qMetaTypeId<T>(), key);
}

template<typename T, typename K>
GenericTask<bool> AsyncDataStore::remove(const K &key)
{
	return internalRemove(qMetaTypeId<T>(), QVariant::fromValue(key).toString());
}

template<typename T>
GenericTask<QList<T>> AsyncDataStore::search(const QString &query)
{
	return internalSearch(qMetaTypeId<T>(), qMetaTypeId<QList<T>>(), query);
}

}

#endif // QTDATASYNC_ASYNCDATASTORE_H