#pragma once
#include <QString>

/**
 * @brief 查询项类
 * 表示音乐文件的元数据信息，包括标题、艺术家、专辑等属性
 */
class QueryItem {
	QString title;              // 音乐标题
	QString artist;             // 艺术家名称
	QString album;              // 专辑名称
	QString genre;              // 音乐流派
	uint year;                  // 发行年份
	uint track;                 // 音轨编号
	QString fileName;           // 文件名（包含路径）
	static int sensitivity;     // 相似度阈值，用于判断两个音乐是否相同

public:
	/**
	 * @brief 默认构造函数
	 */
	explicit QueryItem() = default;
	
	/**
	 * @brief 从文件名构造QueryItem
	 * @param fileName 音乐文件路径，将自动读取文件的元数据
	 */
	explicit QueryItem(const QString& fileName);
	
	/**
	 * @brief 使用完整参数构造QueryItem
	 * @param title 音乐标题
	 * @param artist 艺术家名称
	 * @param album 专辑名称
	 * @param genre 音乐流派
	 * @param year 发行年份
	 * @param track 音轨编号
	 * @param fileName 文件名
	 */
	explicit QueryItem(const QString& title, const QString& artist, const QString& album, 
	                  const QString& genre, uint year, uint track, const QString& fileName);
	
	/**
	 * @brief 获取音乐标题
	 * @return 音乐标题
	 */
	[[nodiscard]] QString getTitle() const;
	
	/**
	 * @brief 获取艺术家名称
	 * @return 艺术家名称
	 */
	[[nodiscard]] QString getArtist() const;
	
	/**
	 * @brief 获取专辑名称
	 * @return 专辑名称
	 */
	[[nodiscard]] QString getAlbum() const;
	
	/**
	 * @brief 获取音乐流派
	 * @return 音乐流派
	 */
	[[nodiscard]] QString getGenre() const;
	
	/**
	 * @brief 获取发行年份
	 * @return 发行年份
	 */
	[[nodiscard]] uint getYear() const;
	
	/**
	 * @brief 获取音轨编号
	 * @return 音轨编号
	 */
	[[nodiscard]] uint getTrack() const;
	
	/**
	 * @brief 获取文件名
	 * @return 文件名（包含路径）
	 */
	[[nodiscard]] QString getFileName() const;
	
	/**
	 * @brief 获取相似度阈值
	 * @return 当前相似度阈值
	 */
	[[nodiscard]] static int getSensitivity() { return sensitivity; }
	
	/**
	 * @brief 设置相似度阈值
	 * @param s 新的相似度阈值，默认为3
	 */
	static void setSensitivity(const int s = 3) { sensitivity = s; }
	
	/**
	 * @brief 设置音乐标题
	 * @param title 新的音乐标题
	 */
	void setTitle(const QString& title);
	
	/**
	 * @brief 设置艺术家名称
	 * @param artist 新的艺术家名称
	 */
	void setArtist(const QString& artist);
	
	/**
	 * @brief 设置专辑名称
	 * @param album 新的专辑名称
	 */
	void setAlbum(const QString& album);
	
	/**
	 * @brief 设置音乐流派
	 * @param genre 新的音乐流派
	 */
	void setGenre(const QString& genre);
	
	/**
	 * @brief 设置发行年份
	 * @param year 新的发行年份
	 */
	void setYear(uint year);
	
	/**
	 * @brief 设置音轨编号
	 * @param track 新的音轨编号
	 */
	void setTrack(uint track);
	
	/**
	 * @brief 设置文件名
	 * @param fileName 新的文件名
	 */
	void setFileName(const QString& fileName);
	
	/**
	 * @brief 相等比较运算符
	 * 根据相似度阈值判断两个音乐项是否相同
	 * @param other 要比较的另一个QueryItem对象
	 * @return 如果相似度达到阈值返回true，否则返回false
	 */
	bool operator==(const QueryItem& other) const;
};
