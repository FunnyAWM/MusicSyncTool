#include "QueryItem.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>

#include <QVector>
#include "../Core/MSTTagUtils.h"

#include "../Core/StringSimilarity.h"

// 默认相似度阈值为5（核心字段 title/artist 各权重2，需至少一个辅助字段匹配）
int QueryItem::similarityThreshold = 5;

/**
 * @brief 从音乐文件构造QueryItem对象
 * 使用TagLib库读取音乐文件的元数据信息
 * @param fileName 音乐文件路径
 */
QueryItem::QueryItem(const QString& fileName) {
	const TagLib::FileRef file = MSTTagUtils::createFileRef(fileName);
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

QString QueryItem::getTitle() const {
	return title;
}

QString QueryItem::getArtist() const {
	return artist;
}

QString QueryItem::getAlbum() const {
	return album;
}

QString QueryItem::getGenre() const {
	return genre;
}

uint QueryItem::getYear() const {
	return year;
}

uint QueryItem::getTrack() const {
	return track;
}

QString QueryItem::getFileName() const {
	return fileName;
}

void QueryItem::setTitle(const QString& title_) {
	this->title = title_;
}

void QueryItem::setArtist(const QString& artist_) {
	this->artist = artist_;
}

void QueryItem::setAlbum(const QString& album_) {
	this->album = album_;
}

void QueryItem::setGenre(const QString& genre_) {
	this->genre = genre_;
}

void QueryItem::setYear(const uint year_) {
	this->year = year_;
}

void QueryItem::setTrack(const uint track_) {
	this->track = track_;
}

void QueryItem::setFileName(const QString& fileName_) {
	this->fileName = fileName_;
}

bool QueryItem::operator==(const QueryItem& other) const {
	int threshold = 0; // 相同字段计数器
	bool hasCoreMatch = false; // 是否有核心字段匹配

	// 核心字段：标题和艺术家（使用模糊匹配，权重为2）
	if (StringSimilarity::fuzzyMatch(title, other.title)) {
		threshold += 2; // 标题匹配权重为2
		hasCoreMatch = true;
	}
	if (StringSimilarity::fuzzyMatch(artist, other.artist)) {
		threshold += 2; // 艺术家匹配权重为2
		hasCoreMatch = true;
	}

	// 辅助字段：模糊匹配，权重为1
	if (StringSimilarity::fuzzyMatch(album, other.album)) {
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
	if (StringSimilarity::fuzzyMatch(genre, other.genre)) {
		threshold++;
	}

	// 判断条件：必须有核心字段匹配且总分达到阈值
	return hasCoreMatch && threshold >= similarityThreshold;
}
