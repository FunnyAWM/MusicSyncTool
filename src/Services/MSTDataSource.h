#ifndef MSTDATASOURCE_H
#define MSTDATASOURCE_H

#include <QMutex>
#include <QDateTime>
#include <QHash>
#include <QSqlQuery>
#include <QString>
#include <QVariant>
#include <type_traits>

#include "../Core/MusicProperties.h"
#include "../Data/QueryItem.h"

/// @brief 收藏状态批量更新项
struct FavoriteUpdate {
	QString fileName;
	bool isFavorite;
};

/// @brief 规则命中状态批量更新项
struct RuleHitUpdate {
	QString fileName;
	bool isRuleHit;
};

using PROPERTIES::OrderByEnum;
using PROPERTIES::QueryRows;
using PROPERTIES::SortByEnum;

class MSTDataSource final : public QObject {
	Q_OBJECT

	QString path;
	QString connection;
	QSqlDatabase db;
	/**
	 * @brief 串行化数据库访问
	 * @details 扫描走 QtConcurrent 工作线程，与 GUI 线程共用同一 SQLite 连接；
	 *          所有访问都必须持有该锁，事务边界（批量写入）必须整体持锁
	 */
	mutable QMutex dbMutex;
	/**
	 * @brief 每线程独立的查询对象，避免共享 QSqlQuery 实例成员导致的数据竞争
	 * @details 不能使用 QThreadStorage：主线程的线程局部数据在 QThreadStorage 对象销毁时不会被清理，
	 *          槽位 id 会被后续实例复用，新实例会直接取到上一个实例遗留的查询对象——它绑定在已关闭的连接上
	 *          （表现为 "QSqlQuery::exec: database not open"，或把语句发到错误的库）。
	 *          这里改为按线程 id 索引并随连接关闭显式释放。
	 */
	mutable QMutex threadQueryMutex;
	mutable QHash<Qt::HANDLE, QSqlQuery*> threadQueries;
	int pageSize = 200;
	int lastFavoriteCount = 0;

	/**
	 * @brief 取当前线程的查询对象
	 * @details 调用方必须已持有 dbMutex（低层 prepare/bind/exec 接口除外，见其说明）
	 */
	[[nodiscard]] QSqlQuery& threadQuery() const;

	/// @brief 释放所有线程的查询对象；关闭连接与析构前调用，避免留下绑定到已关闭连接的查询
	void releaseThreadQueries();

	/// @brief 写入单条记录（读取标签后插入），调用方必须已持有 dbMutex
	[[nodiscard]] bool insertMusic(const QString& file);

	/// @brief 转义 LIKE 模式中的通配符，避免用户输入的 % 与 _ 被当作通配符
	[[nodiscard]] static QString escapeLikePattern(const QString& text);

	/// @brief 打开（或复用）指定目录下的 musicInfo.db，调用方必须已持有 dbMutex
	[[nodiscard]] bool openDatabaseFile(const QString& dir);

	/// @brief 读取 SQLite user_version，作为 schema 版本号
	[[nodiscard]] int userVersion();

	/// @brief 写入 SQLite user_version
	void setUserVersion(int version);

	/// @brief 旧库缺少列时补列，幂等；调用方必须已持有 dbMutex
	[[nodiscard]] bool ensureColumn(const QString& tableName, const QString& columnName, const QString& definition);

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

	~MSTDataSource() override;

	void setPath(const QString& path);

	[[nodiscard]] QString getPath() const { return path; }

	[[nodiscard]] bool isOpen() const { return db.isOpen(); }

	void setConnectionName(const QString& connectionName);

	[[nodiscard]] bool openDB(const QString& path_);

	[[nodiscard]] bool openDB();

	void initTable();

	void closeDB();

	/**
	 * @brief 准备 SQL 语句
	 * @details 低层接口，供构建一次性语句（测试或维护脚本）使用。
	 *          仅适用于单线程调用序列：prepare -> bindValue -> execQuery
	 */
	void prepareStatement(const QString& sqlStatement);

	/**
	 * @brief 绑定参数值到当前线程的预处理语句
	 * @tparam T 参数类型
	 * @param placeholder 参数占位符
	 * @param value 参数值
	 */
	template <class T>
	void bindValue(const QString& placeholder, const T& value) {
		QSqlQuery& query = threadQuery();
		if constexpr (std::is_convertible_v<const T&, QVariant>) {
			query.bindValue(placeholder, value);
		}
		else {
			query.bindValue(placeholder, QVariant::fromValue(value));
		}
	}

	/// @brief 执行当前线程的预处理语句，返回是否成功
	[[nodiscard]] bool execQuery();

	void updateFavoriteBatch(const QList<FavoriteUpdate>& updates);

	void updateRuleHitBatch(const QList<RuleHitUpdate>& updates);

	/**
	 * @brief 批量写入新音乐（单事务、整体持锁）
	 * @param files 待写入的文件名列表
	 * @param failedFiles 输出参数：写入失败的文件名
	 * @return 全部写入成功返回 true；失败时已回滚并返回 false
	 * @details 批量写入必须在同一把锁内完成后，其他线程的语句才不会混入事务边界
	 */
	[[nodiscard]] bool addMusicBatch(const QStringList& files, QStringList& failedFiles);

	QList<QueryItem> getByFileNames(const QStringList& fileNames);

	QList<QueryItem> getAll(const QVector<QueryRows>& rows = {QueryRows::ALL});

	/**
	 * @brief 取出库中全部文件名（只读一列）
	 * @details 扫描阶段的目录/数据库比对只需要文件名。走 getAll 会为每首多构造
	 *          5 个空字符串字段并实例化整个 QueryItem，大曲库下这一列足以造成明显内存放大
	 */
	[[nodiscard]] QStringList getAllFileNames();

	QStringList addMusic(const QStringList& files);

	[[nodiscard]] bool addMusic(const QString& file);

	QList<QueryItem> searchMusic(const QString& text);

	/**
	 * @brief 分页搜索
	 * @param text 搜索文本
	 * @param pageNum 页码（从 1 开始）
	 * @details 命中结果在 SQL 层用 LIMIT/OFFSET 截断，避免整库命中结果进入内存
	 */
	QList<QueryItem> searchMusic(const QString& text, unsigned short pageNum);

	/// @brief 搜索命中总数（用于分页计算）
	[[nodiscard]] int getSearchCount(const QString& text);

	QStringList getFileNameByMetadata(const QList<QueryItem>& items);

	QList<QueryItem> getFavorite(unsigned short pageNum, SortByEnum sortBy, OrderByEnum orderBy);

	QList<QueryItem> getRuleHit(unsigned short pageNum, SortByEnum sortBy, OrderByEnum orderBy);

	QList<QueryItem> getMusicToTable(unsigned short pageNum, SortByEnum sortBy, OrderByEnum orderBy);

	int getCount();

	[[nodiscard]] bool deleteMusic(const QStringList& fileList);

	[[nodiscard]] bool getRuleHit(const QString& fileName);

signals:
	/// @brief 总条目数；跨线程排队投递，参数用 int 保证元类型在 moc 侧可解析
	void totalSize(int size);

	void currentProgress(int progress);
};

#endif // MSTDATASOURCE_H
