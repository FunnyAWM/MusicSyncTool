#include "LyricIgnoreRule.h"

#include <QCoreApplication>
#include <utility>

using namespace PROPERTIES;

LyricIgnoreRule::LyricIgnoreRule(const RuleType ruleType, const RuleField ruleField,
                                 QString ruleName)
	: ruleType(ruleType), ruleField(ruleField), ruleName(std::move(ruleName)) {
}

[[nodiscard]] RuleType LyricIgnoreRule::getRuleType() const {
	return ruleType;
}

[[nodiscard]] RuleField LyricIgnoreRule::getRuleField() const {
	return ruleField;
}

[[nodiscard]] QString LyricIgnoreRule::getRuleName() const {
	return ruleName;
}

/**
 * @brief 将歌词规则字段转换为中文字符串
 * @param rules 规则字段枚举值
 * @return 对应的中文字符串
 */
QString LyricIgnoreRule::lyricRulesToString(const RuleField rules) {
	// 类已改为纯值类型，不能再用 tr()；上下文保持 "LyricIgnoreRule" 以复用既有译文。
	// 这里必须写字面量，否则 lupdate 无法抽取这些文案
	switch (rules) {
	case RuleField::TITLE:
		return QCoreApplication::translate("LyricIgnoreRule", "名称");      // 标题
	case RuleField::ARTIST:
		return QCoreApplication::translate("LyricIgnoreRule", "艺术家");    // 艺术家
	case RuleField::ALBUM:
		return QCoreApplication::translate("LyricIgnoreRule", "专辑");      // 专辑
	default:    // 含 RuleField::NONE 与非法值
		return "";
	}
}

/**
 * @brief 将忽略规则类型转换为中文字符串
 * @param rules 规则类型枚举值
 * @return 对应的中文字符串
 */
QString LyricIgnoreRule::ignoreRulesToString(const RuleType rules) {
	switch (rules) {
	case RuleType::INCLUDES:
		return QCoreApplication::translate("LyricIgnoreRule", "包含");      // 包含匹配
	case RuleType::EXCLUDES:
		return QCoreApplication::translate("LyricIgnoreRule", "排除");      // 排除匹配
	default:
		return "";
	}
}

/**
 * @brief 将中文字符串转换为忽略规则类型
 * @param rule 规则字符串
 * @return 对应的规则类型枚举值
 */
RuleType LyricIgnoreRule::stringToIgnoreRules(const QString& rule) {
	if (rule == "包含") {
		return RuleType::INCLUDES;
	}
	if (rule == "排除") {
		return RuleType::EXCLUDES;
	}
	return RuleType::INCLUDES;  // 默认返回包含类型
}

/**
 * @brief 将中文字符串转换为歌词规则字段
 * @param rule 规则字符串
 * @return 对应的规则字段枚举值
 */
RuleField LyricIgnoreRule::stringToLyricRules(const QString& rule) {
	if (rule == "名称") {
		return RuleField::TITLE;
	}
	if (rule == "艺术家") {
		return RuleField::ARTIST;
	}
	if (rule == "专辑") {
		return RuleField::ALBUM;
	}
	return RuleField::TITLE;  // 默认返回标题字段
}

/**
 * @brief 设置规则的字符串表示
 * 根据枚举值设置对应的中文字符串
 */
void LyricIgnoreRule::setRulesStr() {
	// 设置规则类型字符串
	switch (ruleType) {
	case RuleType::INCLUDES:
		ruleTypeStr = "包含";
		break;
	case RuleType::EXCLUDES:
		ruleTypeStr = "排除";
		break;
	default:    // 含 RuleType::NONE 与非法值
		ruleTypeStr = "";
		break;
	}
	
	// 设置规则字段字符串
	switch (ruleField) {
	case RuleField::TITLE:
		ruleFieldStr = "名称";
		break;
	case RuleField::ARTIST:
		ruleFieldStr = "艺术家";
		break;
	case RuleField::ALBUM:
		ruleFieldStr = "专辑";
		break;
	default:    // 含 RuleField::NONE 与非法值
		ruleFieldStr = "";
		break;
	}
}

QString LyricIgnoreRule::getRuleTypeStr() const {
	return ruleTypeStr;
}

QString LyricIgnoreRule::getRuleFieldStr() const {
	return ruleFieldStr;
}
