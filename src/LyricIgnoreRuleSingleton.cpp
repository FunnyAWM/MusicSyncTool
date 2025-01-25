#include "LyricIgnoreRuleSingleton.h"

LyricIgnoreRuleSingleton::LyricIgnoreRuleSingleton(const RuleType ruleType, const RuleField ruleField,
                                                   QString ruleName)
	: ruleType(ruleType), ruleField(ruleField), ruleName(std::move(ruleName)) {
}

LyricIgnoreRuleSingleton::LyricIgnoreRuleSingleton(const LyricIgnoreRuleSingleton& ruleSingleton) {
	ruleType = ruleSingleton.ruleType;
	ruleField = ruleSingleton.ruleField;
	ruleName = ruleSingleton.ruleName;
}

LyricIgnoreRuleSingleton& LyricIgnoreRuleSingleton::operator=(const LyricIgnoreRuleSingleton&) {
	return *this;
}

[[nodiscard]] RuleType LyricIgnoreRuleSingleton::getRuleType() const {
	return ruleType;
}

[[nodiscard]] RuleField LyricIgnoreRuleSingleton::getRuleField() const {
	return ruleField;
}

[[nodiscard]] QString LyricIgnoreRuleSingleton::getRuleName() const {
	return ruleName;
}

QString LyricIgnoreRuleSingleton::lyricRulesToString(const RuleField rules) {
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

QString LyricIgnoreRuleSingleton::ignoreRulesToString(const RuleType rules) {
	switch (rules) {
	case RuleType::INCLUDES:
		return tr("包含");
	case RuleType::EXCLUDES:
		return tr("排除");
	default:
		return QString();
	}
}

RuleType LyricIgnoreRuleSingleton::stringToIgnoreRules(const QString& rule) {
	if (rule == "包含") {
		return RuleType::INCLUDES;
	}
	if (rule == "排除") {
		return RuleType::EXCLUDES;
	}
	return RuleType::INCLUDES;
}

RuleField LyricIgnoreRuleSingleton::stringToLyricRules(const QString& rule) {
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

void LyricIgnoreRuleSingleton::setRulesStr() {
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

QString LyricIgnoreRuleSingleton::getRuleTypeStr() const {
	return ruleTypeStr;
}

QString LyricIgnoreRuleSingleton::getRuleFieldStr() const {
	return ruleFieldStr;
}
