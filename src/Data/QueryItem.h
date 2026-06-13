/**
 * @file QueryItem.h
 * @brief 音乐查询项类定义
 * @details 定义了表示音乐文件元数据信息的QueryItem类，
 *          包括标题、艺术家、专辑、流派、年份、音轨和文件名等属性，
 *          以及基于相似度阈值的智能比较功能
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef QUERYITEM_H
#define QUERYITEM_H
#include <QString>

/**
 * @brief 查询项类
 * 表示音乐文件的元数据信息，包括标题、艺术家、专辑等属性。
 * 支持基于模糊匹配的智能相等比较，用于音乐去重检测
 */
class QueryItem {
	QString title;              ///< 音乐标题
	QString artist;             ///< 艺术家名称
	QString album;              ///< 专辑名称
	QString genre;              ///< 音乐流派
	uint year;                  ///< 发行年份
	uint track;                 ///< 音轨编号
	QString fileName;           ///< 文件名（包含路径）
	static int similarityThreshold;     ///< 相似度阈值，用于判断两个音乐是否相同

public:
	/**
	 * @brief 默认构造函数
	 */
	explicit QueryItem() = default;
	
	explicit QueryItem(const QString& fileName);
	
	explicit QueryItem(const QString& title, const QString& artist, const QString& album, 
	                  const QString& genre, uint year, uint track, const QString& fileName);
	
	[[nodiscard]] QString getTitle() const;
	
	[[nodiscard]] QString getArtist() const;
	
	[[nodiscard]] QString getAlbum() const;
	
	[[nodiscard]] QString getGenre() const;
	
	[[nodiscard]] uint getYear() const;
	
	[[nodiscard]] uint getTrack() const;
	
	[[nodiscard]] QString getFileName() const;
	
	/**
	 * @brief 获取相似度阈值
	 * @return 当前相似度阈值
	 */
	[[nodiscard]] static int getSimilarityThreshold() { return similarityThreshold; }
	
	/**
	 * @brief 设置相似度阈值
	 * @param s 新的相似度阈值，默认为3
	 */
	static void setSimilarityThreshold(const int threshold = 3) { similarityThreshold = threshold; }
	
	void setTitle(const QString& title_);
	
	void setArtist(const QString& artist_);
	
	void setAlbum(const QString& album_);
	
	void setGenre(const QString& genre_);
	
	void setYear(uint year_);
	
	void setTrack(uint track_);
	
	void setFileName(const QString& fileName_);
	
	bool operator==(const QueryItem& other) const;
};

#endif // QUERYITEM_H
