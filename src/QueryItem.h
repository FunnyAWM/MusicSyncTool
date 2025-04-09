#pragma once
#include <inttypes.h>
#include <QString>
#include <taglib/tag.h>

class QueryItem {
private:
	QString title;
	QString artist;
	QString album;
	QString genre;
	uint year;
	uint track;
	QString fileName;
public:
	explicit QueryItem() = default;
	explicit QueryItem(const QString&);
	explicit QueryItem(const QString&, const QString&, const QString&, const QString&, uint, uint, const QString&);
	QString getTitle() const;
	QString getArtist() const;
	QString getAlbum() const;
	QString getGenre() const;
	uint getYear() const;
	uint getTrack() const;
	QString getFileName() const;
	void setTitle(const QString&);
	void setArtist(const QString&);
	void setAlbum(const QString&);
	void setGenre(const QString&);
	void setYear(const uint);
	void setTrack(const uint);
	void setFileName(const QString&);
};
