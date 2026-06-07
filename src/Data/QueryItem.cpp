/**
 * @file QueryItem.cpp
 * @brief 音乐查询项类的实现
 * @details 实现QueryItem类的构造函数、属性访问和智能比较功能
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "QueryItem.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>
#include <vector>
#include <algorithm>

#include "../Core/MSTTagUtils.h"
#include <QVector>

#include "../Core/StringSimilarity.h"

/// 默认相似度阈值为4（考虑核心字段权重为2）
int QueryItem::sensitivity = 4;

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

// ==================== Getter方法实现 ====================

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

// ==================== Setter方法实现 ====================

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

// ==================== 比较运算符实现 ====================

/**
 * @brief 相等比较运算符
 * @param other 要比较的另一个QueryItem对象
 * @return 如果相似度达到阈值返回true，否则返回false
 * @details 通过比较多个字段来判断两个音乐项是否相似。
 *          核心字段（标题、艺术家）使用模糊匹配且权重为2，
 *          辅助字段（专辑、音轨、年份、流派）权重为1，
 *          必须有核心字段匹配且总分达到sensitivity阈值才认为相等
 */
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
	return hasCoreMatch && threshold >= sensitivity;
}
