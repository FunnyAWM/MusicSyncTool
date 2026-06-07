/**
 * @file MSTTagScanner.h
 * @brief 音乐标签扫描器类定义
 * @details 封装TagLib标签读取、收藏状态判定和规则命中评估逻辑，
 *          将标签扫描职责从数据源中分离，仅负责读取和判定，
 *          不负责数据库写入
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef MSTTAGSCANNER_H
#define MSTTAGSCANNER_H

#include <QList>
#include <QObject>
#include <QString>

#include "../Data/LyricIgnoreRule.h"
#include "../Data/QueryItem.h"

/**
 * @brief 收藏状态更新结果
 * @details 包含单个音乐文件的收藏判定结果
 */
struct FavoriteUpdate {
	QString fileName; ///< 音乐文件名
	bool isFavorite;  ///< 是否包含收藏标签
};

/**
 * @brief 规则命中更新结果
 * @details 包含单个音乐文件的规则命中判定结果
 */
struct RuleHitUpdate {
	QString fileName; ///< 音乐文件名
	bool isRuleHit;   ///< 是否命中规则
};

/**
 * @brief 音乐标签扫描器
 * @details 负责读取音乐文件的TagLib标签，判定收藏状态和规则命中，
 *          返回结果列表供调用方写入数据库。不直接依赖MSTDataSource，
 *          所有需要的数据通过参数传入
 */
class MSTTagScanner : public QObject {
	Q_OBJECT

public:
	explicit MSTTagScanner(QObject* parent = nullptr);
	~MSTTagScanner() override = default;

	/**
	 * @brief 扫描收藏状态
	 * @param basePath 音乐目录路径
	 * @param fileNames 需要扫描的文件名列表
	 * @param tag 收藏标签名称
	 * @return 收藏状态更新结果列表
	 */
	QList<FavoriteUpdate> scanFavorite(const QString& basePath,
	                                   const QStringList& fileNames,
	                                   const QString& tag);

	/**
	 * @brief 扫描规则命中状态
	 * @param basePath 音乐目录路径
	 * @param items 需要扫描的音乐数据列表
	 * @param rules 歌词忽略规则列表
	 * @return 规则命中更新结果列表
	 */
	QList<RuleHitUpdate> scanRuleHit(const QString& basePath,
	                                 const QList<QueryItem>& items,
	                                 const QList<LyricIgnoreRule>& rules);

signals:
	/**
	 * @brief 加载开始信号
	 */
	void loadStarted();

	/**
	 * @brief 总大小信号
	 * @param size 待处理文件总数
	 */
	void totalSize(qsizetype size);

	/**
	 * @brief 当前进度信号
	 * @param progress 当前处理文件索引
	 */
	void currentProgress(qsizetype progress);

	/**
	 * @brief 加载完成信号
	 */
	void loadFinished();
};

#endif // MSTTAGSCANNER_H
