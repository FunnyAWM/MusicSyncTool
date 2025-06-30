#pragma once

#include <QDateTime>
#include <QSqlQuery>
#include <QString>

#include "LyricIgnoreRule.h"
#include "QueryItem.h"

/**
 * @brief 音乐同步工具数据源类
 * 负责管理音乐数据库的连接、查询和操作
 */
class MSTDataSource final : public QObject {

    Q_OBJECT

    QString path;           // 数据库文件路径
    QString connection;     // 数据库连接名称
    QSqlDatabase db;        // 数据库对象
    QSqlQuery query;        // SQL查询对象
    int pageSize = 200;     // 分页大小，默认200条记录

public:
    /**
     * @brief 获取分页大小
     * @return 当前分页大小
     */
    [[nodiscard]] int getPageSize() const;
    
    /**
     * @brief 设置分页大小
     * @param page_size 新的分页大小
     */
    void setPageSize(int page_size);
    
    /**
     * @brief 带路径参数的构造函数
     * @param path 数据库文件路径
     */
    explicit MSTDataSource(const QString& path);
    
    /**
     * @brief 默认构造函数
     */
    explicit MSTDataSource() = default;
    
    /**
     * @brief 析构函数
     */
    ~MSTDataSource() override = default;
    
    /**
     * @brief 设置数据库路径
     * @param path 数据库文件路径
     */
    void setPath(const QString& path);
    
    /**
     * @brief 获取数据库路径
     * @return 当前数据库路径
     */
    [[nodiscard]] QString getPath() const { return path; }
    
    /**
     * @brief 检查数据库是否已打开
     * @return 如果数据库已打开返回true，否则返回false
     */
    [[nodiscard]] bool isOpen() const { return db.isOpen(); }
    
    /**
     * @brief 设置数据库连接名称
     * @param connectionName 连接名称
     */
    void setConnectionName(const QString& connectionName);
    
    /**
     * @brief 打开指定路径的数据库
     * @param path_ 数据库文件路径
     * @return 成功打开返回true，否则返回false
     */
    [[nodiscard]] bool openDB(const QString& path_);
    
    /**
     * @brief 打开数据库（使用已设置的路径）
     * @return 成功打开返回true，否则返回false
     */
    [[nodiscard]] bool openDB();
    
    /**
     * @brief 初始化数据库表结构
     */
    void initTable();
    
    /**
     * @brief 关闭数据库连接
     */
    void closeDB();
    
    /**
     * @brief 准备SQL语句
     * @param stmt SQL语句
     */
    void prepareStatement(const QString& stmt);
    
    /**
     * @brief 绑定参数值到预处理语句
     * @tparam T 参数类型
     * @param placeholder 参数占位符
     * @param value 参数值
     */
    template <class T>
    void bindValue(const QString& placeholder, const T& value);
    
    /**
     * @brief 执行查询
     */
    void execQuery();
    
    /**
     * @brief 设置音乐为收藏
     * @param tag 音乐标签
     * @param timeFromLog 日志时间
     */
    void setFavorite(const QString& tag, const QDateTime& timeFromLog);
    
    /**
     * @brief 设置规则命中
     * @param rules 忽略规则列表
     * @param timeFromLog 日志时间
     */
    void setRuleHit(const QList<LyricIgnoreRule>& rules, const QDateTime& timeFromLog);
    
    /**
     * @brief 获取所有音乐数据
     * @param rows 查询行类型，默认为所有
     * @return 查询结果列表
     */
    QList<QueryItem> getAll(const QVector<QueryRows>& rows = {QueryRows::ALL});
    
    /**
     * @brief 添加音乐文件列表
     * @param files 音乐文件路径列表
     * @return 添加失败的文件列表
     */
    QStringList addMusic(const QStringList& files);
    
    /**
     * @brief 添加单个音乐文件
     * @param file 音乐文件路径
     * @return 成功添加返回true，否则返回false
     */
    [[nodiscard]] bool addMusic(const QString& file);
    
    /**
     * @brief 搜索音乐
     * @param text 搜索关键词
     * @return 搜索结果列表
     */
    QList<QueryItem> searchMusic(const QString& text);
    
    /**
     * @brief 根据查询项获取文件名
     * @param items 查询项列表
     * @return 文件名列表
     */
    QStringList getFileNameByMD(const QList<QueryItem>& items);
    
    /**
     * @brief 获取收藏音乐
     * @param pageNum 页码
     * @param sortBy 排序字段
     * @param orderBy 排序顺序
     * @return 收藏音乐列表
     */
    QList<QueryItem> getFavorite(unsigned short pageNum, SortByEnum sortBy, OrderByEnum orderBy);
    
    /**
     * @brief 获取规则命中的音乐
     * @param pageNum 页码
     * @param sortBy 排序字段
     * @param orderBy 排序顺序
     * @return 规则命中的音乐列表
     */
    QList<QueryItem> getRuleHit(unsigned short pageNum, SortByEnum sortBy, OrderByEnum orderBy);
    
    /**
     * @brief 获取音乐表格数据
     * @param pageNum 页码
     * @param sortBy 排序字段
     * @param orderBy 排序顺序
     * @return 音乐数据列表
     */
    QList<QueryItem> getMusicToTable(unsigned short pageNum, SortByEnum sortBy, OrderByEnum orderBy);
    
    /**
     * @brief 获取音乐总数
     * @return 音乐总数
     */
    int getCount();
    
    /**
     * @brief 删除音乐
     * @param files 要删除的音乐文件列表
     * @return 成功删除返回true，否则返回false
     */
    [[nodiscard]] bool deleteMusic(const QStringList& files);

signals:
    /**
     * @brief 加载开始信号
     */
    void loadStarted();
    
    /**
     * @brief 加载完成信号
     */
    void loadFinished();
    
    /**
     * @brief 加载完成信号（带操作类型）
     * @param operation 操作类型
     */
    void loadFinished(OperationType operation);
    
    /**
     * @brief 总大小信号
     * @param size 总大小
     */
    void totalSize(qsizetype size);
    
    /**
     * @brief 当前进度信号
     * @param progress 当前进度
     */
    void currentProgress(qsizetype progress);
};
