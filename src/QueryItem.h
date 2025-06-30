#pragma once
#include <QString>

class QueryItem {
	QString title;
	QString artist;
	QString album;
	QString genre;
	uint year;
	uint track;
	QString fileName;
	static int sensitivity;

public:
	explicit QueryItem() = default;
	explicit QueryItem(const QString&);
	explicit QueryItem(const QString&, const QString&, const QString&, const QString&, uint, uint, const QString&);
	[[nodiscard]] QString getTitle() const;
	[[nodiscard]] QString getArtist() const;
	[[nodiscard]] QString getAlbum() const;
	[[nodiscard]] QString getGenre() const;
	[[nodiscard]] uint getYear() const;
	[[nodiscard]] uint getTrack() const;
	[[nodiscard]] QString getFileName() const;
	[[nodiscard]] static int getSensitivity() { return sensitivity; }
	static void setSensitivity(const int s = 3) { sensitivity = s; }
	void setTitle(const QString&);
	void setArtist(const QString&);
	void setAlbum(const QString&);
	void setGenre(const QString&);
	void setYear(uint);
	void setTrack(uint);
	void setFileName(const QString&);
	bool operator==(const QueryItem& other) const;
};
