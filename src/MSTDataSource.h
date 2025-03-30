#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <QRegularExpression>
// ReSharper disable once CppUnusedIncludeDirective
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

#include "LyricIgnoreRule.h"
#include "QueryItem.h"

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
    template<class T>
	void bindValue(const QString&, const T&);
	void execQuery();
    void setFavorite(const QString&);
    void setRuleHit(const QList<LyricIgnoreRule>&);
	QList<QueryItem> getAll();
	QStringList addMusic(const QStringList&);
	QList<QueryItem> searchMusic(const QString&);
	QStringList getFileName(const QList<QueryItem>&);
	bool getFavorite(const QueryItem&);
	bool getRuleHit(const QueryItem&);
	[[nodiscard]] bool deleteMusic(const QStringList&);
};
