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
	QTableWidget* localTable;
	QTableWidget* remoteTable;
	QLabel* localPageLabel;
	QLabel* remotePageLabel;
	MSTDataSource& localDataSource;
	MSTDataSource& remoteDataSource;

	unsigned short currentPage[2];
	unsigned short totalPage[2];
	bool favoriteOnly[2];
	QString searchText[2];

	[[nodiscard]] static int pathIndex(PathType path);

	[[nodiscard]] QTableWidget* getTable(PathType path) const;

	[[nodiscard]] QLabel* getPageLabel(PathType path) const;

	[[nodiscard]] MSTDataSource& getDataSource(PathType path) const;

	static void fillTableWithItems(QTableWidget* table,
	                               const QList<QueryItem>& items,
	                               int count);

	void loadSearchPage(PathType path);

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
