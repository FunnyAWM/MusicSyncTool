#pragma once
#include <QObject>
#include <QString>

#include "MusicProperties.h"
using namespace PROPERTIES;

class LyricIgnoreRuleSingleton final : public QObject {
	ignoreLyricRules ruleType;
	lyricRules ruleField;
	QString ruleName;

public:
	explicit LyricIgnoreRuleSingleton(ignoreLyricRules, lyricRules, QString);
	LyricIgnoreRuleSingleton(const LyricIgnoreRuleSingleton&);
	LyricIgnoreRuleSingleton& operator=(const LyricIgnoreRuleSingleton&);
	~LyricIgnoreRuleSingleton() override = default;
	[[nodiscard]] ignoreLyricRules getRuleType() const;
	[[nodiscard]] lyricRules getRuleField() const;
	[[nodiscard]] QString getRuleName() const;
	static QString lyricRulesToString(lyricRules);
	static QString ignoreRulesToString(ignoreLyricRules);
	static ignoreLyricRules stringToIgnoreRules(const QString);
	static lyricRules stringToLyricRules(const QString);
};
