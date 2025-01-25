#pragma once
#include <QObject>
#include <QString>

#include "MusicProperties.h"
using namespace PROPERTIES;

class LyricIgnoreRuleSingleton final : public QObject {
	Q_OBJECT
	RuleType ruleType;
	RuleField ruleField;
	QString ruleName;
	QString ruleTypeStr;
	QString ruleFieldStr;

public:
	explicit LyricIgnoreRuleSingleton(RuleType, RuleField, QString);
	LyricIgnoreRuleSingleton(const LyricIgnoreRuleSingleton&);
	LyricIgnoreRuleSingleton& operator=(const LyricIgnoreRuleSingleton&);
	~LyricIgnoreRuleSingleton() override = default;
	[[nodiscard]] RuleType getRuleType() const;
	[[nodiscard]] RuleField getRuleField() const;
	[[nodiscard]] QString getRuleName() const;
	static QString lyricRulesToString(RuleField);
	static QString ignoreRulesToString(RuleType);
	static RuleType stringToIgnoreRules(const QString&);
	static RuleField stringToLyricRules(const QString&);
	void setRulesStr();
	bool operator==(const LyricIgnoreRuleSingleton& other) const {
		return ruleType == other.ruleType && ruleField == other.ruleField && ruleName == other.ruleName;
	}

	bool operator!=(const LyricIgnoreRuleSingleton& other) const {
		return !(*this == other);
	}
	QString getRuleTypeStr() const;
	QString getRuleFieldStr() const;
};
