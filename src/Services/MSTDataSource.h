/**
 * @file MSTDataSource.h
 * @brief 音乐同步工具数据源类定义
 * @details 定义了管理音乐数据库连接、查询和操作的数据源类，
 *          封装了SQLite数据库的CRUD操作，包括音乐文件的添加、删除、
 *          搜索、分页查询以及收藏和规则命中状态管理
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef MSTDATASOURCE_H
#define MSTDATASOURCE_H

#include <QDateTime>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <type_traits>

#include "MSTTagScanner.h"
#include "../Data/QueryItem.h"

/**
 * @brief 音乐同步工具数据源类
 * 负责管理音乐数据库的连接、查询和操作。
 * 继承自QObject以支持信号槽机制进行异步进度通知
 */
class MSTDataSource final : public QObject {
	Q_OBJECT

	QString path; ///< 数据库文件路径
	QString connection; ///< 数据库连接名称
	QSqlDatabase db; ///< 数据库对象
	QSqlQuery query; ///< SQL查询对象
	int pageSize = 200; ///< 分页大小，默认200条记录
	int lastFavoriteCount = 0; ///< 最近一次getFavorite查询的收藏总数

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
