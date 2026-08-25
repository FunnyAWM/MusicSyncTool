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
	bool ignoreLyric;
	bool recursiveScan;
	short sortBy;
	short orderBy;
	QString language;
	QString favoriteTag;
	QList<LyricIgnoreRule> rules;
};
#endif // SETTINGENTITY_H
