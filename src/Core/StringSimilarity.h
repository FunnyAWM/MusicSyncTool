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
#include <vector>

/**
 * @brief 字符串相似度计算命名空间
 * 包含所有字符串匹配和相似度计算的算法实现
 */
namespace StringSimilarity {

	std::vector<int> buildKMPNext(const QString& pattern);

	bool kmpSimilarity(const QString& text, const QString& pattern, double threshold = 0.8);

	bool isCJKChar(const QChar& ch);

	bool isCJKString(const QString& str);

	QChar fullWidthToHalfWidth(const QChar& ch);

	QString normalizeCJKString(const QString& str);

	double cjkStringSimilarity(const QString& str1, const QString& str2);

	bool fuzzyMatch(const QString& str1, const QString& str2);

} // namespace StringSimilarity

#endif // STRINGSIMILARITY_H
