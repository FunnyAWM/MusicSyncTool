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

	/// @brief 使用 KMP 判断相似性；默认阈值与 StringSimilarity.cpp 中的 similarityThreshold 保持一致
	bool kmpSimilarity(const QString& text, const QString& pattern, double threshold = 0.85);

	bool isCJKChar(const QChar& ch);

	bool isCJKString(const QString& str);

	QChar fullWidthToHalfWidth(const QChar& ch);

	QString normalizeCJKString(const QString& str);

	double cjkStringSimilarity(const QString& str1, const QString& str2);

	bool fuzzyMatch(const QString& str1, const QString& str2);

} // namespace StringSimilarity

#endif // STRINGSIMILARITY_H
