#include "QueryItem.h"

#include <taglib/fileref.h>
#include <taglib/tag.h>

QueryItem::QueryItem(const QString& fileName) {
	const TagLib::FileRef file(fileName.toStdWString().c_str());
	if (file.isNull()) {
		return;
	}
	const TagLib::Tag* tag = file.tag();
	title = tag->title().toCString();
	artist = tag->artist().toCString();
	album = tag->album().toCString();
	genre = tag->genre().toCString();
	year = tag->year();
	track = tag->track();
	this->fileName = fileName;
}

QueryItem::QueryItem(const QString& title, const QString& artist, const QString& album, const QString& genre,
                     const uint year, const uint track, const QString& fileName):
	year(0), track(0) {
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
