#include "StringSimilarity.h"
#include <algorithm>
#include <QVector>

namespace StringSimilarity {

	// 默认相似度阈值
	constexpr float similarityThreshold = 0.85f;

	/**
	 * @brief 构建KMP算法的next数组（失效函数）
	 * @param pattern 模式串
	 * @return next数组
	 */
	std::vector<int> buildKMPNext(const QString& pattern) {
		const int m = pattern.length();
		std::vector<int> next(m, 0);
		int j = 0;

		for (int i = 1; i < m; ++i) {
			while (j > 0 && pattern[i] != pattern[j]) {
				j = next[j - 1];
			}
			if (pattern[i] == pattern[j]) {
				j++;
			}
			next[i] = j;
		}
		return next;
	}

	/**
	 * @brief 使用KMP算法检查字符串相似性
	 * @param text 文本串
	 * @param pattern 模式串
	 * @param threshold 相似度阈值(0.0-1.0)
	 * @return 是否相似
	 */
	bool kmpSimilarity(const QString& text, const QString& pattern, double threshold) {
		if (text.isEmpty() || pattern.isEmpty()) {
			return text.isEmpty() && pattern.isEmpty();
		}

		// 如果完全相同，直接返回true
		if (text == pattern) {
			return true;
		}

		// 将字符串转换为小写进行比较
		const QString lowerText = text.toLower();
		const QString lowerPattern = pattern.toLower();

		if (lowerText == lowerPattern) {
			return true;
		}

		// 计算较短字符串与较长字符串的匹配程度
		QString shorter = (lowerText.length() <= lowerPattern.length()) ? lowerText : lowerPattern;
		QString longer = (lowerText.length() > lowerPattern.length()) ? lowerText : lowerPattern;

		const std::vector<int> next = buildKMPNext(shorter);
		int matchCount = 0;
		int j = 0;

		for (int i = 0; i < longer.length(); ++i) {
			while (j > 0 && longer[i] != shorter[j]) {
				j = next[j - 1];
			}
			if (longer[i] == shorter[j]) {
				j++;
				matchCount++;
				if (j == shorter.length()) {
					// 找到完整匹配
					return true;
				}
			}
		}

		// 计算相似度：匹配字符数 / 较短字符串长度
		double similarity = static_cast<double>(matchCount) / shorter.length();
		return similarity >= threshold;
	}

	/**
	 * @brief 检查字符是否为CJK字符
	 * @param ch 输入字符
	 * @return 如果是CJK字符返回true
	 */
	bool isCJKChar(const QChar& ch) {
		const int32_t unicode = ch.unicode();
		return (unicode >= 0x4E00 && unicode <= 0x9FFF) || // CJK统一汉字
			(unicode >= 0x3400 && unicode <= 0x4DBF) || // CJK扩展A
			(unicode >= 0x20000 && unicode <= 0x2A6DF) || // CJK扩展B
			(unicode >= 0x2A700 && unicode <= 0x2B73F) || // CJK扩展C
			(unicode >= 0x2B740 && unicode <= 0x2B81F) || // CJK扩展D
			(unicode >= 0x3040 && unicode <= 0x309F) || // 平假名
			(unicode >= 0x30A0 && unicode <= 0x30FF) || // 片假名
			(unicode >= 0xAC00 && unicode <= 0xD7AF) || // 韩文字母
			(unicode >= 0x1100 && unicode <= 0x11FF) || // 韩文字母扩展
			(unicode >= 0xFF00 && unicode <= 0xFFEF); // 全角字符
	}

	/**
	 * @brief 检查字符串是否包含CJK字符
	 * @param str 输入字符串
	 * @return 如果包含CJK字符返回true
	 */
	bool isCJKString(const QString& str) {
		return std::any_of(str.begin(), str.end(), [](const QChar& ch) {
			return isCJKChar(ch);
		});
	}

	/**
	 * @brief 全角字符转半角字符
	 * @param ch 输入字符
	 * @return 转换后的字符
	 */
	QChar fullWidthToHalfWidth(const QChar& ch) {
		const ushort unicode = ch.unicode();

		// 全角ASCII字符 (FF01-FF5E) 转换为半角 (0021-007E)
		if (unicode >= 0xFF01 && unicode <= 0xFF5E) {
			return QChar(unicode - 0xFEE0);
		}

		// 全角空格转换为半角空格
		if (unicode == 0x3000) {
			return QChar(0x0020);
		}

		// 其他字符保持不变
		return ch;
	}

	/**
	 * @brief 标准化CJK字符串
	 * @param str 输入字符串
	 * @return 标准化后的字符串
	 */
	QString normalizeCJKString(const QString& str) {
		QString normalized;
		normalized.reserve(str.length());

		for (const QChar& ch : str) {
			QChar converted = fullWidthToHalfWidth(ch);

			// 跳过某些标点符号和空白字符（但保留基本空格）
			if (!converted.isSpace() || converted == QChar(' ')) {
				// 对于CJK字符，保持原样；对于ASCII字符，转换为小写
				if (isCJKChar(converted)) {
					normalized.append(converted);
				}
				else {
					normalized.append(converted.toLower());
				}
			}
		}

		return normalized.simplified(); // 移除首尾空格并合并连续空格
	}

	/**
	 * @brief CJK字符串的相似度计算（基于字符级编辑距离）
	 * @param str1 第一个字符串
	 * @param str2 第二个字符串
	 * @return 相似度值 (0.0-1.0)
	 */
	double cjkStringSimilarity(const QString& str1, const QString& str2) {
		if (str1.isEmpty() && str2.isEmpty()) {
			return 1.0;
		}

		if (str1.isEmpty() || str2.isEmpty()) {
			return 0.0;
		}

		// 使用动态规划计算编辑距离（Levenshtein距离）
		const int len1 = str1.length();
		const int len2 = str2.length();

		// 创建DP表
		QVector<QVector<int>> dp(len1 + 1, QVector<int>(len2 + 1));

		// 初始化
		for (int i = 0; i <= len1; ++i) {
			dp[i][0] = i;
		}
		for (int j = 0; j <= len2; ++j) {
			dp[0][j] = j;
		}

		// 填充DP表
		for (int i = 1; i <= len1; ++i) {
			for (int j = 1; j <= len2; ++j) {
				if (str1[i - 1] == str2[j - 1]) {
					dp[i][j] = dp[i - 1][j - 1];
				}
				else {
					int min_val = qMin(dp[i - 1][j], dp[i][j - 1]);
					min_val = qMin(min_val, dp[i - 1][j - 1]);
					dp[i][j] = 1 + min_val;
				}
			}
		}

		const int editDistance = dp[len1][len2];
		const int maxLen = qMax(len1, len2);

		// 计算相似度：1 - (编辑距离 / 最大长度)
		return 1.0 - (static_cast<double>(editDistance) / maxLen);
	}

	/**
	 * @brief 检查两个字符串是否模糊匹配（CJK优化版本）
	 * @param str1 字符串1
	 * @param str2 字符串2
	 * @return 是否匹配
	 */
	bool fuzzyMatch(const QString& str1, const QString& str2) {
		// 空字符串不参与匹配，避免无元数据文件互相误报
		if (str1.isEmpty() || str2.isEmpty()) {
			return false;
		}

		// 先检查精确匹配
		if (str1 == str2) {
			return true;
		}

		// 检查是否包含CJK字符
		const bool hasCJK1 = isCJKString(str1);
		const bool hasCJK2 = isCJKString(str2);

		if (hasCJK1 || hasCJK2) {
			// CJK字符串处理
			const QString norm1 = normalizeCJKString(str1);
			const QString norm2 = normalizeCJKString(str2);

			// 标准化后精确匹配
			if (norm1 == norm2) {
				return true;
			}

			// 使用CJK特殊相似度算法（更严格的阈值0.85）
			const double similarity = cjkStringSimilarity(norm1, norm2);
			return similarity >= 0.85;
		}
		else {
			// 非CJK字符串使用KMP算法
			return kmpSimilarity(str1, str2, similarityThreshold);
		}
	}

} // namespace StringSimilarity
