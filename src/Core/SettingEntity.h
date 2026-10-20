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
 * @details 所有成员都带默认值：settings.json 缺键或解析失败时不会留下未初始化的数据
 */
struct SettingsData {
	bool ignoreLyric{false};                                    ///< 是否忽略歌词检查
	bool recursiveScan{false};                                  ///< 是否递归扫描子目录
	short sortBy{PROPERTIES::toShort(PROPERTIES::SortByEnum::TITLE)};   ///< 排序字段编码
	short orderBy{PROPERTIES::toShort(PROPERTIES::OrderByEnum::ASC)};   ///< 升降序编码
	QString language;                                           ///< 界面语言
	QString favoriteTag;                                        ///< 自定义"喜欢的音乐"标签
	QList<LyricIgnoreRule> rules;                               ///< 自定义歌词忽略规则
	short themeMode{PROPERTIES::toShort(PROPERTIES::ThemeMode::LIGHT)};   ///< 明暗模式编码
	short accentColor{PROPERTIES::toShort(PROPERTIES::AccentColor::GREEN)}; ///< 强调色编码
	bool logToFile{false};                                      ///< 是否将日志写入文件
};
#endif // SETTINGENTITY_H
