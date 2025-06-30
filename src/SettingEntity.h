#ifndef SETTINGENTITY_H
#define SETTINGENTITY_H
#include <QString>
#include "MusicProperties.h"
using PROPERTIES::RuleField;
using PROPERTIES::RuleType;

/**
 * @brief 设置实体结构体
 * 包含应用程序的所有设置信息
 */
struct set {
	bool ignoreLyric;                    // 是否忽略歌词处理
	short sortBy;                        // 排序字段（标题/艺术家/专辑）
	short orderBy;                       // 排序顺序（升序/降序）
	QString language;                    // 界面语言设置
	QString favoriteTag;                 // 收藏标签
	QList<LyricIgnoreRule> rules;        // 歌词忽略规则列表
};
#endif // SETTINGENTITY_H
