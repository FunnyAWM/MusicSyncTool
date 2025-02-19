#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>

class MSTDataSource final :
	public QObject {
	Q_OBJECT

private:
	QString path;
	QSqlDatabase db;
	QSqlQuery query;
public:
	explicit MSTDataSource(const QString&);
	~MSTDataSource() override = default;
	void openDB(const QString&);
	void closeDB();
	void prepareStatement(const QString&);
	void bindValue(const QString&, const QString&);
	void execQuery();
	void getAll(const QStringList& cols);
	QStringList addMusic(const QStringList&);
	bool deleteMusic(const QStringList&);
};
