/**
 * @file SettingEntity.h
 * @brief 应用程序设置实体定义
 * @details 定义了包含应用程序所有配置信息的设置结构体，
 *          包括歌词忽略、排序方式、语言、收藏标签和忽略规则等
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef SETTINGENTITY_H
#define SETTINGENTITY_H
#include <QString>
#include "MusicProperties.h"
#include "../Data/LyricIgnoreRule.h"
using PROPERTIES::RuleField;
using PROPERTIES::RuleType;

/**
 * @brief 设置实体结构体
 * 包含应用程序的所有用户可配置信息，用于在界面和持久化层之间传递设置数据
 */
struct SettingsData {
	bool ignoreLyric;                    ///< 是否忽略歌词处理
	bool recursiveScan;                  ///< 是否递归扫描子目录
	short sortBy;                        ///< 排序字段（标题/艺术家/专辑）
	short orderBy;                       ///< 排序顺序（升序/降序）
	QString language;                    ///< 界面语言设置
	QString favoriteTag;                 ///< 收藏标签名称
	QList<LyricIgnoreRule> rules;        ///< 歌词忽略规则列表
};
#endif // SETTINGENTITY_H
