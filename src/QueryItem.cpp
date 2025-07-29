#include "QueryItem.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <vector>
#include <algorithm>
#include <QVector>

// KMP算法相关辅助函数
namespace {
	constexpr float similarityThreshold = 0.8; // 默认相似度阈值
	
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
	bool kmpSimilarity(const QString& text, const QString& pattern, double threshold = 0.8) {
		if (text.isEmpty() || pattern.isEmpty()) {
			return text.isEmpty() && pattern.isEmpty();
		}

		// 如果完全相同，直接返回true
		if (text == pattern) {
			return true;
		}

		// 将字符串转换为小写进行比较
		QString lowerText = text.toLower();
		QString lowerPattern = pattern.toLower();

		if (lowerText == lowerPattern) {
			return true;
		}

		// 计算较短字符串与较长字符串的匹配程度
		QString shorter = (lowerText.length() <= lowerPattern.length()) ? lowerText : lowerPattern;
		QString longer = (lowerText.length() > lowerPattern.length()) ? lowerText : lowerPattern;

		std::vector<int> next = buildKMPNext(shorter);
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
		ushort unicode = ch.unicode();
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
		for (const QChar& ch : str) {
			if (isCJKChar(ch)) {
				return true;
			}
		}
		return false;
	}

	/**
	 * @brief 全角字符转半角字符
	 * @param ch 输入字符
	 * @return 转换后的字符
	 */
	QChar fullWidthToHalfWidth(const QChar& ch) {
		ushort unicode = ch.unicode();

		// 全角ASCII字符 (FF01-FF5E) 转换为半角 (0021-007E)
		if (unicode >= 0xFF01 && unicode <= 0xFF5E) {
			return QChar(unicode - 0xFEE0);
		}

		// 全角空格转换为半角空格
		if (unicode == 0x3000) {
			return QChar(0x0020);
		}

		// 特殊全角字符转换
		switch (unicode) {
		case 0xFF0F:
			return QChar('/'); // 全角斜杠
		case 0xFF1A:
			return QChar(':'); // 全角冒号
		case 0xFF1B:
			return QChar(';'); // 全角分号
		case 0xFF1F:
			return QChar('?'); // 全角问号
		case 0xFF01:
			return QChar('!'); // 全角感叹号
		case 0xFF0C:
			return QChar(','); // 全角逗号
		case 0xFF0E:
			return QChar('.'); // 全角句号
		default:
			return ch;
		}
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
		int len1 = str1.length();
		int len2 = str2.length();

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

		int editDistance = dp[len1][len2];
		int maxLen = qMax(len1, len2);

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
		// 先检查精确匹配
		if (str1 == str2) {
			return true;
		}

		// 检查是否包含CJK字符
		bool hasCJK1 = isCJKString(str1);
		bool hasCJK2 = isCJKString(str2);

		if (hasCJK1 || hasCJK2) {
			// CJK字符串处理
			QString norm1 = normalizeCJKString(str1);
			QString norm2 = normalizeCJKString(str2);

			// 标准化后精确匹配
			if (norm1 == norm2) {
				return true;
			}

			// 使用CJK特殊相似度算法
			double similarity = cjkStringSimilarity(norm1, norm2);

			// CJK字符使用更严格的阈值（0.85）
			return similarity >= 0.85;
		}
		else {
			// 非CJK字符串使用原有KMP算法
			return kmpSimilarity(str1, str2, similarityThreshold);
		}
	}
}

int QueryItem::sensitivity = 4; // 默认相似度阈值为4（考虑核心字段权重为2）

/**
 * @brief 从音乐文件构造QueryItem对象
 * 使用TagLib库读取音乐文件的元数据信息
 * @param fileName 音乐文件路径
 */
QueryItem::QueryItem(const QString& fileName) {
#if defined(_WIN64) or defined(_WIN32)
	const TagLib::FileRef file(fileName.toStdWString().c_str()); // Windows平台使用宽字符
#else
	const TagLib::FileRef file(fileName.toStdString().c_str()); // 其他平台使用标准字符
#endif
	if (file.isNull()) {
		return; // 如果文件无效则直接返回
	}
	const TagLib::Tag* tag = file.tag(); // 获取文件标签信息

	// 从标签中提取各种元数据
	title = QString::fromStdString(tag->title().to8Bit(true));
	artist = QString::fromStdString(tag->artist().to8Bit(true));
	album = QString::fromStdString(tag->album().to8Bit(true));
	genre = QString::fromStdString(tag->genre().to8Bit(true));
	year = tag->year();
	track = tag->track();
	this->fileName = fileName;
}

/**
 * @brief 使用完整参数构造QueryItem对象
 * @param title 音乐标题
 * @param artist 艺术家名称
 * @param album 专辑名称
 * @param genre 音乐流派
 * @param year 发行年份
 * @param track 音轨编号
 * @param fileName 文件名
 */
QueryItem::QueryItem(const QString& title, const QString& artist, const QString& album, const QString& genre,
                     const uint year, const uint track, const QString& fileName) : year(0), track(0) {
	// 初始化数值成员为0
	this->title = title;
	this->artist = artist;
	this->album = album;
	this->genre = genre;
	this->year = year;
	this->track = track;
	this->fileName = fileName;
}

// Getter方法实现
/**
 * @brief 获取音乐标题
 * @return 音乐标题字符串
 */
QString QueryItem::getTitle() const {
	return title;
}

/**
 * @brief 获取艺术家名称
 * @return 艺术家名称字符串
 */
QString QueryItem::getArtist() const {
	return artist;
}

/**
 * @brief 获取专辑名称
 * @return 专辑名称字符串
 */
QString QueryItem::getAlbum() const {
	return album;
}

/**
 * @brief 获取音乐流派
 * @return 音乐流派字符串
 */
QString QueryItem::getGenre() const {
	return genre;
}

/**
 * @brief 获取发行年份
 * @return 发行年份
 */
uint QueryItem::getYear() const {
	return year;
}

/**
 * @brief 获取音轨编号
 * @return 音轨编号
 */
uint QueryItem::getTrack() const {
	return track;
}

/**
 * @brief 获取文件名
 * @return 文件名字符串（包含完整路径）
 */
QString QueryItem::getFileName() const {
	return fileName;
}

// Setter方法实现
/**
 * @brief 设置音乐标题
 * @param title 新的音乐标题
 */
void QueryItem::setTitle(const QString& title) {
	this->title = title;
}

/**
 * @brief 设置艺术家名称
 * @param artist 新的艺术家名称
 */
void QueryItem::setArtist(const QString& artist) {
	this->artist = artist;
}

/**
 * @brief 设置专辑名称
 * @param album 新的专辑名称
 */
void QueryItem::setAlbum(const QString& album) {
	this->album = album;
}

/**
 * @brief 设置音乐流派
 * @param genre 新的音乐流派
 */
void QueryItem::setGenre(const QString& genre) {
	this->genre = genre;
}

/**
 * @brief 设置发行年份
 * @param year 新的发行年份
 */
void QueryItem::setYear(const uint year) {
	this->year = year;
}

/**
 * @brief 设置音轨编号
 * @param track 新的音轨编号
 */
void QueryItem::setTrack(const uint track) {
	this->track = track;
}

/**
 * @brief 设置文件名
 * @param fileName 新的文件名
 */
void QueryItem::setFileName(const QString& fileName) {
	this->fileName = fileName;
}

/**
 * @brief 相等比较运算符
 * 通过比较多个字段来判断两个音乐项是否相似
 * 当相同字段数量达到或超过敏感度阈值时，认为两个音乐项相等
 * @param other 要比较的另一个QueryItem对象
 * @return 如果相似度达到阈值返回true，否则返回false
 */
bool QueryItem::operator==(const QueryItem& other) const {
	int threshold = 0; // 相同字段计数器
	bool hasCoreMatch = false; // 是否有核心字段匹配

	// 核心字段：标题和艺术家（使用KMP算法进行模糊匹配）
	if (fuzzyMatch(title, other.title)) {
		threshold += 2; // 标题匹配权重为2
		hasCoreMatch = true;
	}
	if (fuzzyMatch(artist, other.artist)) {
		threshold += 2; // 艺术家匹配权重为2
		hasCoreMatch = true;
	}

	// 辅助字段：使用模糊匹配，权重为1
	if (fuzzyMatch(album, other.album)) {
		threshold++;
	}
	if (track == other.track) {
		// 音轨号精确匹配
		threshold++;
	}
	if (year == other.year) {
		// 年份精确匹配
		threshold++;
	}
	if (fuzzyMatch(genre, other.genre)) {
		threshold++;
	}

	// 判断条件：必须有核心字段匹配且总分达到阈值
	return hasCoreMatch && threshold >= sensitivity;
}
