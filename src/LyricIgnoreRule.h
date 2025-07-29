#pragma once
#include <QObject>
#include <QString>

#include "MusicProperties.h"
using namespace PROPERTIES;  // NOLINT(clang-diagnostic-header-hygiene)

/**
 * @brief 歌词忽略规则类
 * 用于定义和管理歌词处理的忽略规则，包括规则类型、规则字段和规则名称
 */
class LyricIgnoreRule final : public QObject {
	Q_OBJECT
	
	RuleType ruleType;      // 规则类型（包含/排除）
	RuleField ruleField;    // 规则字段（标题/艺术家/专辑）
	QString ruleName;       // 规则名称/值
	QString ruleTypeStr;    // 规则类型字符串表示
	QString ruleFieldStr;   // 规则字段字符串表示

public:
	/**
	 * @brief 构造函数
	 * @param ruleType 规则类型
	 * @param ruleField 规则字段
	 * @param ruleName 规则名称
	 */
	explicit LyricIgnoreRule(RuleType ruleType, RuleField ruleField, QString ruleName);
	
	/**
	 * @brief 拷贝构造函数
	 * @param ruleSingleton 要拷贝的规则对象
	 */
	LyricIgnoreRule(const LyricIgnoreRule& ruleSingleton);
	
	/**
	 * @brief 赋值运算符
	 * @param other 要赋值的规则对象
	 * @return 返回当前对象引用
	 */
	LyricIgnoreRule& operator=(const LyricIgnoreRule& other);
	
	/**
	 * @brief 析构函数
	 */
	~LyricIgnoreRule() override = default;
	
	/**
	 * @brief 获取规则类型
	 * @return 规则类型枚举值
	 */
	[[nodiscard]] RuleType getRuleType() const;
	
	/**
	 * @brief 获取规则字段
	 * @return 规则字段枚举值
	 */
	[[nodiscard]] RuleField getRuleField() const;
	
	/**
	 * @brief 获取规则名称
	 * @return 规则名称字符串
	 */
	[[nodiscard]] QString getRuleName() const;
	
	/**
	 * @brief 将歌词规则字段转换为字符串
	 * @param rules 规则字段枚举
	 * @return 对应的中文字符串
	 */
	static QString lyricRulesToString(RuleField rules);
	
	/**
	 * @brief 将忽略规则类型转换为字符串
	 * @param rules 规则类型枚举
	 * @return 对应的中文字符串
	 */
	static QString ignoreRulesToString(RuleType rules);
	
	/**
	 * @brief 将字符串转换为忽略规则类型
	 * @param rule 规则字符串
	 * @return 对应的规则类型枚举
	 */
	static RuleType stringToIgnoreRules(const QString& rule);
	
	/**
	 * @brief 将字符串转换为歌词规则字段
	 * @param rule 规则字符串
	 * @return 对应的规则字段枚举
	 */
	static RuleField stringToLyricRules(const QString& rule);
	
	/**
	 * @brief 设置规则字符串表示
	 * 根据枚举值设置对应的中文字符串
	 */
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
	
	/**
	 * @brief 获取规则类型字符串
	 * @return 规则类型的中文字符串表示
	 */
	QString getRuleTypeStr() const;
	
	/**
	 * @brief 获取规则字段字符串
	 * @return 规则字段的中文字符串表示
	 */
	QString getRuleFieldStr() const;
};
