#include "LyricIgnoreRuleSingleton.h"

LyricIgnoreRuleSingleton::LyricIgnoreRuleSingleton(const ignoreLyricRules ruleType, const lyricRules ruleField,
                                                   QString ruleName)
	: ruleType(ruleType), ruleField(ruleField), ruleName(std::move(ruleName)) {
}

LyricIgnoreRuleSingleton::LyricIgnoreRuleSingleton(const LyricIgnoreRuleSingleton& ruleSingleton) {
	ruleType = ruleSingleton.ruleType;
	ruleField = ruleSingleton.ruleField;
	ruleName = ruleSingleton.ruleName;
}

LyricIgnoreRuleSingleton& LyricIgnoreRuleSingleton::operator=(const LyricIgnoreRuleSingleton&)
{
	return *this;
}

[[nodiscard]] ignoreLyricRules LyricIgnoreRuleSingleton::getRuleType() const {
	return ruleType;
}

[[nodiscard]] lyricRules LyricIgnoreRuleSingleton::getRuleField() const {
	return ruleField;
}

[[nodiscard]] QString LyricIgnoreRuleSingleton::getRuleName() const {
	return ruleName;
}

QString LyricIgnoreRuleSingleton::lyricRulesToString(const lyricRules rules) {
	switch (static_cast<lyricRules>(rules)) {
	case lyricRules::NAME:
		return QString(tr("名称"));
	case lyricRules::ARTIST:
		return QString(tr("艺术家"));
	case lyricRules::ALBUM:
		return QString(tr("专辑"));
	default:
		return QString();
	}
}

QString LyricIgnoreRuleSingleton::ignoreRulesToString(const ignoreLyricRules rules) {
	switch (rules) {
	case ignoreLyricRules::INCLUDES:
		return QString(tr("包含"));
	case ignoreLyricRules::EXCLUDES:
		return QString(tr("排除"));
	default:
		return QString();
	}
}

ignoreLyricRules LyricIgnoreRuleSingleton::stringToIgnoreRules(const QString rule) {
	if (rule == "包含") {
		return ignoreLyricRules::INCLUDES;
	}
	if (rule == "排除") {
		return ignoreLyricRules::EXCLUDES;
	}
	return ignoreLyricRules::INCLUDES;
}

lyricRules LyricIgnoreRuleSingleton::stringToLyricRules(const QString rule) {
	if (rule == "名称") {
		return lyricRules::NAME;
	}
	if (rule == "艺术家") {
		return lyricRules::ARTIST;
	}
	if (rule == "专辑") {
		return lyricRules::ALBUM;
	}
	return lyricRules::NAME;
}
