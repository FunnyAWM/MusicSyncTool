/**
 * @file StringSimilarity.h
 * @brief 字符串相似度计算模块
 * @details 提供多种字符串相似度匹配算法，包括KMP模式匹配、
 *          CJK字符处理、编辑距离计算和模糊匹配功能。
 *          用于音乐元数据的智能去重和搜索匹配
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef STRINGSIMILARITY_H
#define STRINGSIMILARITY_H

#include <QString>
#include <QChar>
#include <vector>

/**
 * @brief 字符串相似度计算命名空间
 * 包含所有字符串匹配和相似度计算的算法实现
 */
namespace StringSimilarity {

	/**
	 * @brief 构建KMP算法的next数组（失效函数）
	 * @param pattern 模式串
	 * @return next数组，用于KMP匹配过程中的回溯
	 */
	std::vector<int> buildKMPNext(const QString& pattern);

	/**
	 * @brief 使用KMP算法检查字符串相似性
	 * @param text 文本串
	 * @param pattern 模式串
	 * @param threshold 相似度阈值(0.0-1.0)，默认0.8
	 * @return 如果相似度达到阈值返回true，否则返回false
	 */
	bool kmpSimilarity(const QString& text, const QString& pattern, double threshold = 0.8);

	/**
	 * @brief 检查字符是否为CJK字符
	 * @param ch 输入字符
	 * @return 如果是CJK字符（包括中日韩统一汉字、平假名、片假名、韩文等）返回true
	 */
	bool isCJKChar(const QChar& ch);

	/**
	 * @brief 检查字符串是否包含CJK字符
	 * @param str 输入字符串
	 * @return 如果包含至少一个CJK字符返回true
	 */
	bool isCJKString(const QString& str);

	/**
	 * @brief 全角字符转半角字符
	 * @param ch 输入字符
	 * @return 转换后的半角字符，如果无法转换则返回原字符
	 */
	QChar fullWidthToHalfWidth(const QChar& ch);

	/**
	 * @brief 标准化CJK字符串
	 * @param str 输入字符串
	 * @return 标准化后的字符串（全角转半角、统一大小写、去除多余空白）
	 */
	QString normalizeCJKString(const QString& str);

	/**
	 * @brief 计算CJK字符串的相似度（基于字符级编辑距离）
	 * @param str1 第一个字符串
	 * @param str2 第二个字符串
	 * @return 相似度值 (0.0-1.0)，1.0表示完全相同
	 */
	double cjkStringSimilarity(const QString& str1, const QString& str2);

	/**
	 * @brief 检查两个字符串是否模糊匹配（CJK优化版本）
	 * @param str1 字符串1
	 * @param str2 字符串2
	 * @return 如果两字符串模糊匹配返回true
	 * @details 对于CJK字符串使用编辑距离算法，对于非CJK字符串使用KMP算法。
	 *          CJK字符使用更严格的阈值(0.85)以适配中日韩文字的精确匹配需求
	 */
	bool fuzzyMatch(const QString& str1, const QString& str2);

} // namespace StringSimilarity

#endif // STRINGSIMILARITY_H
