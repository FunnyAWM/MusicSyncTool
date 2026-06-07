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
#include <QSet>
#include <QStringList>
#include <QTableWidget>

#include "../../Core/MusicProperties.h"
#include "../../Data/QueryItem.h"

using PROPERTIES::PathType;
using PROPERTIES::PET;

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
	/**
	 * @brief 构造函数
	 * @param localTable 本地音乐表格控件
	 * @param remoteTable 远程音乐表格控件
	 * @param localPageLabel 本地分页标签控件
	 * @param remotePageLabel 远程分页标签控件
	 * @param localDataSource 本地数据源引用
	 * @param remoteDataSource 远程数据源引用
	 * @param parent 父对象指针
	 */
	explicit MSTTableManager(QTableWidget* localTable,
	                         QTableWidget* remoteTable,
	                         QLabel* localPageLabel,
	                         QLabel* remotePageLabel,
	                         MSTDataSource& localDataSource,
	                         MSTDataSource& remoteDataSource,
	                         QObject* parent = nullptr);


	void connectSignals() const;

	/**
	 * @brief 搜索音乐并填充表格
	 * @param path 路径类型（本地或远程）
	 * @param text 搜索关键字
	 * @details 在指定数据源中搜索包含关键字的音乐文件，
	 *          如果关键字为空则重新加载第一页所有音乐
	 */
	void searchMusic(PathType path, const QString& text);

	/**
	 * @brief 获取表格中选中的音乐文件名
	 * @param path 路径类型（本地或远程）
	 * @return 选中音乐文件的文件名列表
	 * @details 从指定表格中提取选中行的标题、艺术家和专辑信息，
	 *          通过数据源查询对应的文件名列表
	 */
	QStringList getSelectedMusic(PathType path) const;

	/**
	 * @brief 加载收藏音乐到表格
	 * @param path 路径类型（本地或远程）
	 * @param page 页码
	 * @param favoriteTag 收藏标签关键字
	 * @param sortBy 排序字段
	 * @param orderBy 排序方向
	 * @details 从数据源获取标记为收藏的音乐文件列表，支持分页显示。
	 *          如果没有设置收藏标签，发出错误信号
	 */
	void getFavoriteMusic(PathType path, unsigned short page,
	                      const QString& favoriteTag,
	                      PROPERTIES::SortByEnum sortBy,
	                      PROPERTIES::OrderByEnum orderBy);

	/**
	 * @brief 填充音乐列表到表格
	 * @param path 路径类型（本地或远程）
	 * @param items 音乐数据列表
	 * @param currentPage 当前页码
	 * @param totalPages 总页数
	 * @details 将给定的音乐数据列表填充到对应的表格控件中，
	 *          并更新分页标签显示
	 */
	void populateTable(PathType path, const QList<QueryItem>& items,
	                   unsigned short currentPage, unsigned short totalPages);

	/**
	 * @brief 尝试翻到上一页
	 * @param path 路径类型（本地或远程）
	 * @param hasPath 是否已选择路径
	 * @param favoriteTag 收藏标签关键字
	 * @param sortBy 排序字段
	 * @param orderBy 排序方向
	 * @details 检查边界条件后翻到上一页，如果已在第一页则发出错误信号
	 */
	void goToPrevPage(PathType path, bool hasPath,
	                  const QString& favoriteTag,
	                  PROPERTIES::SortByEnum sortBy,
	                  PROPERTIES::OrderByEnum orderBy);

	/**
	 * @brief 尝试翻到下一页
	 * @param path 路径类型（本地或远程）
	 * @param hasPath 是否已选择路径
	 * @param favoriteTag 收藏标签关键字
	 * @param sortBy 排序字段
	 * @param orderBy 排序方向
	 * @details 检查边界条件后翻到下一页，如果已在最后一页则发出错误信号
	 */
	void goToNextPage(PathType path, bool hasPath,
	                  const QString& favoriteTag,
	                  PROPERTIES::SortByEnum sortBy,
	                  PROPERTIES::OrderByEnum orderBy);

	/// @brief 获取指定路径的当前页码
	[[nodiscard]] unsigned short getCurrentPage(PathType path) const;

	/// @brief 设置指定路径的当前页码
	void setCurrentPage(PathType path, unsigned short page);

	/// @brief 获取指定路径的总页数
	[[nodiscard]] unsigned short getTotalPage(PathType path) const;

	/// @brief 设置指定路径的总页数
	void setTotalPage(PathType path, unsigned short total);

	/// @brief 获取指定路径是否处于收藏过滤模式
	[[nodiscard]] bool isFavoriteOnly(PathType path) const;

	/// @brief 设置指定路径的收藏过滤模式
	void setFavoriteOnly(PathType path, bool value);

	/// @brief 每页显示的音乐条目数
	static constexpr short PAGESIZE = 200;

signals:
	/**
	 * @brief 错误信号
	 * @param type 错误类型
	 * @details 当分页越界、未选择路径或未设置收藏标签时发出
	 */
	void errorOccurred(PET type);

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

	/**
	 * @brief 将PathType转换为数组索引
	 * @param path 路径类型
	 * @return 0表示本地，1表示远程
	 */
	[[nodiscard]] static int pathIndex(PathType path);

	/**
	 * @brief 获取指定路径对应的表格控件
	 * @param path 路径类型
	 * @return 对应的表格指针
	 */
	[[nodiscard]] QTableWidget* getTable(PathType path) const;

	/**
	 * @brief 获取指定路径对应的分页标签
	 * @param path 路径类型
	 * @return 对应的标签指针
	 */
	[[nodiscard]] QLabel* getPageLabel(PathType path) const;

	/**
	 * @brief 获取指定路径对应的数据源
	 * @param path 路径类型
	 * @return 数据源引用
	 */
	[[nodiscard]] MSTDataSource& getDataSource(PathType path) const;

	/**
	 * @brief 将QueryItem列表填充到表格
	 * @param table 目标表格
	 * @param items 音乐数据列表
	 * @param start 起始索引
	 * @param count 填充条数
	 */
	static void fillTableWithItems(QTableWidget* table,
	                               const QList<QueryItem>& items,
	                               int count);
public slots:
	void scrollToTop(PathType path) const;
};

#endif // MSTTABLEMANAGER_H
