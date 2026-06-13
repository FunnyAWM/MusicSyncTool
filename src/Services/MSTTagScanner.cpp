/**
 * @file MSTTagScanner.cpp
 * @brief 音乐标签扫描器类的实现
 * @details 实现TagLib标签读取、收藏状态判定和规则命中评估
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "MSTTagScanner.h"

#include <QRegularExpression>
#include <taglib/fileref.h>
#include <taglib/tpropertymap.h>

#include "Logger.h"
#include "../Core/MSTTagUtils.h"

/**
 * @brief 构造函数
 */
MSTTagScanner::MSTTagScanner(QObject* parent) : QObject(parent) {
}

/**
 * @brief 扫描收藏状态
 */
QList<FavoriteUpdate> MSTTagScanner::scanFavorite(const QString& basePath,
                                                   const QStringList& fileNames,
                                                   const QString& tag) {
	if (fileNames.isEmpty()) {
		Logger::Info("No music files to scan for favorites.");
		return {};
	}

	emit totalSize(fileNames.count());

	QList<FavoriteUpdate> results;
	int i = 0;
	for (const auto& file : fileNames) {
		emit currentProgress(i);

		// 读取文件标签
		const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(basePath + "/" + file);

		FavoriteUpdate update;
		update.fileName = file;
		update.isFavorite = fileRef.properties().contains(tag.toStdString().c_str());
		results.append(update);
		i++;
	}

	return results;
}

/**
 * @brief 扫描规则命中状态
 */
QList<RuleHitUpdate> MSTTagScanner::scanRuleHit(const QString& basePath,
                                                 const QList<QueryItem>& items,
                                                 const QList<LyricIgnoreRule>& rules) {
	if (items.isEmpty()) {
		Logger::Info("No music items to scan for rule hits.");
		return {};
	}

	QList<RuleHitUpdate> results;
	emit totalSize(items.count());
	int i = 0;
	for (const auto& item : items) {
		RuleHitUpdate update;
		update.fileName = item.getFileName();
		emit currentProgress(i);
		// 检查是否有任何规则命中当前音乐项
		update.isRuleHit = std::any_of(rules.begin(), rules.end(), [&item](const LyricIgnoreRule& rule) {
			QString fieldValue;
			// 根据规则字段类型获取对应的音乐属性
			switch (rule.getRuleField()) {
			case RuleField::TITLE:
				fieldValue = item.getTitle();
				break;
			case RuleField::ARTIST:
				fieldValue = item.getArtist();
				break;
			case RuleField::ALBUM:
				fieldValue = item.getAlbum();
				break;
			}
			// 使用正则表达式匹配规则
			const QRegularExpression regExp(rule.getRuleName());
			const bool hasMatch = regExp.match(fieldValue).hasMatch();
			// 根据规则类型返回匹配结果（包含或排除）
			return (rule.getRuleType() == RuleType::INCLUDES) ? hasMatch : !hasMatch;
		});

		results.append(update);
		i++;
	}

	return results;
}
