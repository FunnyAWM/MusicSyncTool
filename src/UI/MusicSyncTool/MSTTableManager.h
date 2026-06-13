/**
 * @file MSTTableManager.h
 * @brief 音乐表格管理器类定义
 * @details 统一管理本地和远程两个音乐表格的数据填充、分页导航和选择操作，
 *          将表格相关的状态和逻辑从主窗口中分离出来
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef MSTTABLEMANAGER_H
#define MSTTABLEMANAGER_H

#include <QLabel>
#include <QObject>
#include <QStringList>
#include <QTableWidget>

#include "../../Core/MusicProperties.h"
#include "../../Data/QueryItem.h"

using PROPERTIES::PathType;
using PROPERTIES::AppErrorType;

class MSTDataSource;

/**
 * @brief 音乐表格管理器
 * @details 负责本地和远程两个QTableWidget的数据填充、分页状态维护、
 *          搜索结果显示和选中行提取。通过PathType索引区分本地/远程表格，
 *          统一管理页码、收藏过滤等状态
 */
class MSTTableManager : public QObject {
	Q_OBJECT

public:
	explicit MSTTableManager(QTableWidget* localTable,
	                         QTableWidget* remoteTable,
	                         QLabel* localPageLabel,
	                         QLabel* remotePageLabel,
	                         MSTDataSource& localDataSource,
	                         MSTDataSource& remoteDataSource,
	                         QObject* parent = nullptr);

private:
	QTableWidget* localTable;       ///< 本地音乐表格控件
	QTableWidget* remoteTable;      ///< 远程音乐表格控件
	QLabel* localPageLabel;         ///< 本地分页标签
	QLabel* remotePageLabel;        ///< 远程分页标签
	MSTDataSource& localDataSource; ///< 本地数据源引用
	MSTDataSource& remoteDataSource;///< 远程数据源引用

	unsigned short currentPage[2];  ///< 当前页码 [0:本地, 1:远程]
	unsigned short totalPage[2];    ///< 总页数 [0:本地, 1:远程]
	bool favoriteOnly[2];           ///< 收藏过滤模式 [0:本地, 1:远程]

	[[nodiscard]] static int pathIndex(PathType path);

	[[nodiscard]] QTableWidget* getTable(PathType path) const;

	[[nodiscard]] QLabel* getPageLabel(PathType path) const;

	[[nodiscard]] MSTDataSource& getDataSource(PathType path) const;

	static void fillTableWithItems(QTableWidget* table,
	                               const QList<QueryItem>& items,
	                               int count);

public:
	void connectSignals() const;

	void searchMusic(PathType path, const QString& text);

	QStringList getSelectedMusic(PathType path) const;

	void getFavoriteMusic(PathType path, unsigned short page,
	                      const QString& favoriteTag,
	                      PROPERTIES::SortByEnum sortBy,
	                      PROPERTIES::OrderByEnum orderBy);

	void populateTable(PathType path, const QList<QueryItem>& items,
	                   unsigned short currentPage, unsigned short totalPages);

	void goToPrevPage(PathType path, bool hasPath,
	                  const QString& favoriteTag,
	                  PROPERTIES::SortByEnum sortBy,
	                  PROPERTIES::OrderByEnum orderBy);

	void goToNextPage(PathType path, bool hasPath,
	                  const QString& favoriteTag,
	                  PROPERTIES::SortByEnum sortBy,
	                  PROPERTIES::OrderByEnum orderBy);

	[[nodiscard]] unsigned short getCurrentPage(PathType path) const;

	void setCurrentPage(PathType path, unsigned short page);

	[[nodiscard]] unsigned short getTotalPage(PathType path) const;

	void setTotalPage(PathType path, unsigned short total);

	[[nodiscard]] bool isFavoriteOnly(PathType path) const;

	void setFavoriteOnly(PathType path, bool value);

	/// @brief 每页显示的音乐条目数
	static constexpr short PAGESIZE = 200;

signals:
	/**
	 * @brief 错误信号
	 * @param type 错误类型
	 * @details 当分页越界、未选择路径或未设置收藏标签时发出
	 */
	void errorOccurred(AppErrorType type);

	/**
	 * @brief 请求重新加载音乐列表
	 * @param path 路径类型
	 * @param page 页码
	 * @details 翻页或刷新时通知主窗口重新加载数据
	 */
	void requestLoadMusic(PathType path, unsigned short page);

	/**
	 * @brief 请求重新加载收藏列表
	 * @param path 路径类型
	 * @param page 页码
	 * @details 翻页时通知主窗口重新加载收藏数据
	 */
	void requestLoadFavorite(PathType path, unsigned short page);

	void requestScroll(PathType path);

public slots:
	void scrollToTop(PathType path) const;
};

#endif // MSTTABLEMANAGER_H
