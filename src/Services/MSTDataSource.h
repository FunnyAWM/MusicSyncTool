#ifndef MSTDATASOURCE_H
#define MSTDATASOURCE_H

#include <QDateTime>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <type_traits>

#include "MSTTagScanner.h"
#include "../Data/QueryItem.h"

class MSTDataSource final : public QObject {
	Q_OBJECT

	QString path;
	QString connection;
	QSqlDatabase db;
	QSqlQuery query;
	int pageSize = 200;
	int lastFavoriteCount = 0;

public:
	[[nodiscard]] int getPageSize() const;

	/**
	 * @brief 获取最近一次查询的收藏总数
	 * @return 收藏音乐的真实总数（由getFavorite设置）
	 */
	[[nodiscard]] int getLastFavoriteCount() const { return lastFavoriteCount; }

	void setPageSize(int page_size);

	explicit MSTDataSource(const QString& path);

	explicit MSTDataSource() = default;

	~MSTDataSource() override = default;

	void setPath(const QString& path);

	[[nodiscard]] QString getPath() const { return path; }

	[[nodiscard]] bool isOpen() const { return db.isOpen(); }

	void setConnectionName(const QString& connectionName);

	[[nodiscard]] bool openDB(const QString& path_);

	[[nodiscard]] bool openDB();

	void initTable();

	void closeDB();

	void prepareStatement(const QString& sqlStatement);

	/**
	 * @brief 绑定参数值到预处理语句
	 * @tparam T 参数类型
	 * @param placeholder 参数占位符
	 * @param value 参数值
	 */
	template <class T>
	void bindValue(const QString& placeholder, const T& value) {
		if constexpr (std::is_convertible_v<const T&, QVariant>) {
			query.bindValue(placeholder, value);
		}
		else {
			query.bindValue(placeholder, QVariant::fromValue(value));
		}
	}

	void execQuery();

	void updateFavoriteBatch(const QList<FavoriteUpdate>& updates);

	void updateRuleHitBatch(const QList<RuleHitUpdate>& updates);

	void beginTransaction();

	void commitTransaction();

	QList<QueryItem> getByFileNames(const QStringList& fileNames);

	QList<QueryItem> getAll(const QVector<QueryRows>& rows = {QueryRows::ALL});

	QStringList addMusic(const QStringList& files);

	[[nodiscard]] bool addMusic(const QString& file);

	QList<QueryItem> searchMusic(const QString& text);

	QStringList getFileNameByMetadata(const QList<QueryItem>& items);

	QList<QueryItem> getFavorite(unsigned short pageNum, SortByEnum sortBy, OrderByEnum orderBy);

	QList<QueryItem> getRuleHit(unsigned short pageNum, SortByEnum sortBy, OrderByEnum orderBy);

	QList<QueryItem> getMusicToTable(unsigned short pageNum, SortByEnum sortBy, OrderByEnum orderBy);

	int getCount();

	[[nodiscard]] bool deleteMusic(const QStringList& fileList);

	[[nodiscard]] bool getRuleHit(const QString& fileName);

signals:
	void loadStarted();

	void loadFinished();

	void loadFinished(OperationType operation);

	void totalSize(qsizetype size);

	void currentProgress(qsizetype progress);
};

#endif // MSTDATASOURCE_H
