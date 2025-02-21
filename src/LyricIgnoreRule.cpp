#include "LyricIgnoreRule.h"

LyricIgnoreRule::LyricIgnoreRule(const RuleType ruleType, const RuleField ruleField,
                                 QString ruleName)
	: ruleType(ruleType), ruleField(ruleField), ruleName(std::move(ruleName)) {
}

LyricIgnoreRule::LyricIgnoreRule(const LyricIgnoreRule& ruleSingleton) {
	ruleType = ruleSingleton.ruleType;
	ruleField = ruleSingleton.ruleField;
	ruleName = ruleSingleton.ruleName;
}

LyricIgnoreRule& LyricIgnoreRule::operator=(const LyricIgnoreRule&) {
	return *this;
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

QString LyricIgnoreRule::lyricRulesToString(const RuleField rules) {
	switch (rules) {
	case RuleField::TITLE:
		return tr("名称");
	case RuleField::ARTIST:
		return tr("艺术家");
	case RuleField::ALBUM:
		return tr("专辑");
	default:
		return QString();
	}
}

QString LyricIgnoreRule::ignoreRulesToString(const RuleType rules) {
	switch (rules) {
	case RuleType::INCLUDES:
		return tr("包含");
	case RuleType::EXCLUDES:
		return tr("排除");
	default:
		return QString();
	}
}

RuleType LyricIgnoreRule::stringToIgnoreRules(const QString& rule) {
	if (rule == "包含") {
		return RuleType::INCLUDES;
	}
	if (rule == "排除") {
		return RuleType::EXCLUDES;
	}
	return RuleType::INCLUDES;
}

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
	return RuleField::TITLE;
}

void LyricIgnoreRule::setRulesStr() {
	switch (ruleType) {
	case RuleType::INCLUDES:
		ruleTypeStr = "包含";
		break;
	case RuleType::EXCLUDES:
		ruleTypeStr = "排除";
		break;
	}
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
	}
}

QString LyricIgnoreRule::getRuleTypeStr() const {
	return ruleTypeStr;
}

QString LyricIgnoreRule::getRuleFieldStr() const {
	return ruleFieldStr;
}
