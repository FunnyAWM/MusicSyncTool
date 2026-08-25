#ifndef LYRICIGNORERULE_H
#define LYRICIGNORERULE_H
#include <QObject>
#include <QString>

#include "../Core/MusicProperties.h"
using namespace PROPERTIES;  // NOLINT(clang-diagnostic-header-hygiene)

/**
 * @brief 歌词忽略规则类
 * 用于定义和管理歌词处理的忽略规则，包括规则类型、规则字段和规则名称。
 * 支持规则与中文字符串之间的双向转换
 */
class LyricIgnoreRule final : public QObject {
	Q_OBJECT
	
	RuleType ruleType;
	RuleField ruleField;
	QString ruleName;
	QString ruleTypeStr;
	QString ruleFieldStr;

public:
	explicit LyricIgnoreRule(RuleType ruleType, RuleField ruleField, QString ruleName);
	
	LyricIgnoreRule(const LyricIgnoreRule& other);
	
	LyricIgnoreRule& operator=(const LyricIgnoreRule& other);
	
	~LyricIgnoreRule() override = default;
	
	[[nodiscard]] RuleType getRuleType() const;
	
	[[nodiscard]] RuleField getRuleField() const;
	
	[[nodiscard]] QString getRuleName() const;
	
	static QString lyricRulesToString(RuleField rules);
	
	static QString ignoreRulesToString(RuleType rules);
	
	static RuleType stringToIgnoreRules(const QString& rule);
	
	static RuleField stringToLyricRules(const QString& rule);
	
	void setRulesStr();
	
	/**
	 * @brief 相等比较运算符
	 * @param other 要比较的规则对象
	 * @return 如果规则完全相同返回true，否则返回false
	 */
	bool operator==(const LyricIgnoreRule& other) const {
		return ruleType == other.ruleType && ruleField == other.ruleField && ruleName == other.ruleName;
	}

	/**
	 * @brief 不等比较运算符
	 * @param other 要比较的规则对象
	 * @return 如果规则不同返回true，否则返回false
	 */
	bool operator!=(const LyricIgnoreRule& other) const {
		return !(*this == other);
	}
	
	QString getRuleTypeStr() const;
	
	QString getRuleFieldStr() const;
};

#endif // LYRICIGNORERULE_H
