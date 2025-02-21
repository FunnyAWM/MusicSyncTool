#pragma once
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>

#include "src/QueryItem.h"

class MSTDataSource final :
	public QObject {
	Q_OBJECT
	QString path;
	QSqlDatabase db;
	QSqlQuery query;
public:
	explicit MSTDataSource(const QString&);
	~MSTDataSource() override = default;
	void openDB(const QString&);
	void initTable();
	void closeDB();
	void prepareStatement(const QString&);
	void bindValue(const QString&, const QString&);
	void execQuery();
	QList<QueryItem> getAll();
	QStringList addMusic(const QStringList&);
	QList<QueryItem> searchMusic(const QString&);
	QStringList getFileName(const QList<QueryItem>&);
	bool getFavorite(const QueryItem&);
	bool getRuleHit(const QueryItem&);
	[[nodiscard]] bool deleteMusic(const QStringList&);
};
