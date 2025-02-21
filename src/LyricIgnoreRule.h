#pragma once
#include <QObject>
#include <QString>

#include "MusicProperties.h"
using namespace PROPERTIES;

class LyricIgnoreRule final : public QObject {
	Q_OBJECT
	RuleType ruleType;
	RuleField ruleField;
	QString ruleName;
	QString ruleTypeStr;
	QString ruleFieldStr;

public:
	explicit LyricIgnoreRule(RuleType, RuleField, QString);
	LyricIgnoreRule(const LyricIgnoreRule&);
	LyricIgnoreRule& operator=(const LyricIgnoreRule&);
	~LyricIgnoreRule() override = default;
	[[nodiscard]] RuleType getRuleType() const;
	[[nodiscard]] RuleField getRuleField() const;
	[[nodiscard]] QString getRuleName() const;
	static QString lyricRulesToString(RuleField);
	static QString ignoreRulesToString(RuleType);
	static RuleType stringToIgnoreRules(const QString&);
	static RuleField stringToLyricRules(const QString&);
	void setRulesStr();
	bool operator==(const LyricIgnoreRule& other) const {
		return ruleType == other.ruleType && ruleField == other.ruleField && ruleName == other.ruleName;
	}

	bool operator!=(const LyricIgnoreRule& other) const {
		return !(*this == other);
	}
	QString getRuleTypeStr() const;
	QString getRuleFieldStr() const;
};
