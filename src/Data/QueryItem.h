#ifndef QUERYITEM_H
#define QUERYITEM_H
#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>

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

	/**
	 * @brief 在已排序的列表中查找重复项
	 * @details 仅比较相邻项，时间复杂度 O(n)，要求 items 已按标题排序
	 * @param items 已排序的音乐列表
	 * @return 重复项的文件名，成对出现（前一项、后一项）
	 */
	[[nodiscard]] static QStringList findDuplicatesInSorted(const QList<QueryItem>& items);
};

Q_DECLARE_METATYPE(QueryItem)
Q_DECLARE_METATYPE(QList<QueryItem>)

#endif // QUERYITEM_H
