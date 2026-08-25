#ifndef QUERYITEM_H
#define QUERYITEM_H
#include <QString>

/**
 * @brief 查询项类
 * 表示音乐文件的元数据信息，包括标题、艺术家、专辑等属性。
 * 支持基于模糊匹配的智能相等比较，用于音乐去重检测
 */
class QueryItem {
	QString title;
	QString artist;
	QString album;
	QString genre;
	uint year;
	uint track;
	QString fileName;
	static int similarityThreshold;

public:
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
	
	[[nodiscard]] static int getSimilarityThreshold() { return similarityThreshold; }
	
	static void setSimilarityThreshold(const int threshold = 5) { similarityThreshold = threshold; }
	
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
