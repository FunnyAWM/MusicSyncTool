#pragma once
#include <QString>
#include <cinttypes>
#include <taglib/tag.h>

class QueryItem {
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
    [[nodiscard]] QString getTitle() const;
    [[nodiscard]] QString getArtist() const;
    [[nodiscard]] QString getAlbum() const;
    [[nodiscard]] QString getGenre() const;
    [[nodiscard]] uint getYear() const;
    [[nodiscard]] uint getTrack() const;
    [[nodiscard]] QString getFileName() const;
    void setTitle(const QString&);
    void setArtist(const QString&);
    void setAlbum(const QString&);
    void setGenre(const QString&);
    void setYear(uint);
    void setTrack(uint);
    void setFileName(const QString&);
};
