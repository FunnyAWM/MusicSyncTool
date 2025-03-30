#include "MSTDataSource.h"

#include <QFile>
#include <taglib/fileref.h>
#include <tpropertymap.h>

MSTDataSource::MSTDataSource(const QString& path) {
    this->path = path;
    openDB(path + "/musicInfo.db");
}

void MSTDataSource::openDB(const QString& path) {
    db = QSqlDatabase::addDatabase("QSQLITE");
    if (path.length() - path.lastIndexOf('/') == 1) { //如果最后一个字符为'/'
        db.setDatabaseName(path + "musicInfo.db");
    } else {
        db.setDatabaseName(path + "/musicInfo.db");
    }
    if (!db.open()) {
        qWarning() << "[WARN] Error opening database:" << db.lastError().text();
    }
    query = QSqlQuery(db);
}

void MSTDataSource::initTable() {
    query.exec("CREATE TABLE IF NOT EXISTS musicInfo (title TEXT, artist TEXT, album TEXT, genre TEXT, year INT, "
               "track INT, favorite BOOL, ruleHit BOOL, fileName TEXT)");
}

void MSTDataSource::closeDB() {
    if (db.isOpen()) {
        db.close();
    }
}

void MSTDataSource::prepareStatement(const QString& stmt) { query.prepare(stmt); }

template <class T>
void MSTDataSource::bindValue(const QString& key, const T& value) { // 懒得重载所以用模板了
    const auto variantValue = QVariant(value);  // 为了防止挖坑做一下类型检查
    if (!variantValue.isValid()) {
        qFatal("[FATAL] Error in bindValue: Invalid value for key %s", qPrintable(key));
    }
    query.bindValue(key, variantValue);
}

void MSTDataSource::execQuery() { query.exec(); }
void MSTDataSource::setFavorite(const QString& tag) {
    prepareStatement("SELECT fileName FROM musicInfo");
    execQuery();
    QStringList fileName;
    while (query.next()) {
        fileName.append(query.value(0).toString());
    }
    for (auto& file : fileName) {
        TagLib::FileRef f;
#if defined(_WIN64) or defined(_WIN32)
        f = TagLib::FileRef(file.toStdWString().c_str());
#else
        f = TagLib::FileRef(file.toStdString().c_str());
#endif
        prepareStatement("UPDATE musicInfo SET favorite = :fav WHERE title = :title, artist = :artist, album = :album, "
                         "fileName = :fileName");
        bindValue(":fav", f.properties().contains(tag.toStdString()));
        bindValue(":title", f.tag()->title().toCString());
        bindValue(":artist", f.tag()->artist().toCString());
        bindValue(":album", f.tag()->album().toCString());
    }
}
void MSTDataSource::setRuleHit(const QList<LyricIgnoreRule>& rules) {
    QList<QueryItem> items = getAll();
    for (auto& item : items) {
        prepareStatement("UPDATE musicInfo SET "
                         "ruleHit = :ruleHit WHERE "
                         "title = :title, "
                         "artist = :artist, "
                         "album = :album, "
                         "fileName = :fileName");
        bindValue(":ruleHit", std::any_of(rules.begin(), rules.end(), [&item](const LyricIgnoreRule& rule) {
                      QString fieldStr;
                      switch (rule.getRuleField()) {
                      case RuleField::TITLE:
                          fieldStr = item.getTitle();
                          break;
                      case RuleField::ARTIST:
                          fieldStr = item.getArtist();
                          break;
                      case RuleField::ALBUM:
                          fieldStr = item.getAlbum();
                          break;
                      }
                      const QRegularExpression regExp(rule.getRuleName());
                      const bool hasMatch = regExp.match(fieldStr).hasMatch();
                      return (rule.getRuleType() == RuleType::INCLUDES) ? hasMatch : !hasMatch;
                  }));
        bindValue(":title", item.getTitle());
        bindValue(":artist", item.getArtist());
        bindValue(":album", item.getAlbum());
        execQuery();
    }
}

QList<QueryItem> MSTDataSource::getAll() {
    prepareStatement("SELECT title, artist, album, genre, year, track, fileName FROM musicInfo");
    execQuery();
    QList<QueryItem> items;
    while (query.next()) {
        items.append(QueryItem(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(),
                               query.value(3).toString(), query.value(4).toUInt(), query.value(5).toUInt(),
                               query.value(6).toString()));
    }
    return items;
}

QStringList MSTDataSource::addMusic(const QStringList& files) {
    auto errList = QStringList();
    for (const QString& file : files) {
        TagLib::FileRef fileRef;
#if defined(_WIN64) or defined(_WIN32)
        fileRef = TagLib::FileRef(file.toStdWString().c_str());
#else
        fileRef = TagLib::FileRef(file.toStdString().c_str());
#endif
        if (fileRef.isNull()) {
            errList.append(file);
            continue;
        }
        const TagLib::Tag* tag = fileRef.tag();
        prepareStatement("INSERT INTO musicInfo (title, artist, album, genre, year, track, fileName) "
                         "VALUES (:title, :artist, :album, :genre, :year, :track, :fileName)");
        bindValue(":title", tag->title().toCString());
        bindValue(":artist", tag->artist().toCString());
        bindValue(":album", tag->album().toCString());
        bindValue(":genre", tag->genre().toCString());
        bindValue(":year", QString::number(tag->year()));
        bindValue(":track", QString::number(tag->track()));
        bindValue(":fileName", file);
        if (!query.exec()) {
            qWarning() << "[WARN] Error inserting data into database:" << query.lastError().text();
            errList.append(file);
        }
    }
    return errList;
}

QList<QueryItem> MSTDataSource::searchMusic(const QString& text) {
    prepareStatement("SELECT title, artist, album, genre, year, track, fileName FROM musicInfo WHERE title = :text OR "
                     "artist = :text OR album = :text");
    bindValue(":text", text);
    execQuery();
    QList<QueryItem> items;
    while (query.next()) {
        items.append(QueryItem(query.value(0).toString(), query.value(1).toString(), query.value(2).toString(),
                               query.value(3).toString(), query.value(4).toUInt(), query.value(5).toUInt(),
                               query.value(6).toString()));
    }
    return items;
}

QStringList MSTDataSource::getFileName(const QList<QueryItem>& items) {
    QStringList fileList;
    for (const QueryItem& item : items) {
        prepareStatement("SELECT fileName FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album");
        bindValue(":title", item.getTitle());
        bindValue(":artist", item.getArtist());
        bindValue(":album", item.getAlbum());
        execQuery();
        query.next();
        fileList.append(query.value(0).toString());
    }
    return fileList;
}

inline bool MSTDataSource::getFavorite(const QueryItem& item) {
    prepareStatement("SELECT favorite FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album AND "
                     "fileName = :fileName");
    bindValue(":title", item.getTitle());
    bindValue(":artist", item.getArtist());
    bindValue(":album", item.getAlbum());
    bindValue(":fileName", item.getFileName());
    execQuery();
    query.next();
    return query.value(0).toBool();
}

inline bool MSTDataSource::getRuleHit(const QueryItem& item) {
    prepareStatement("SELECT ruleHit FROM musicInfo WHERE title = :title AND artist = :artist AND album = :album AND "
                     "fileName = :fileName");
    bindValue(":title", item.getTitle());
    bindValue(":artist", item.getArtist());
    bindValue(":album", item.getAlbum());
    bindValue(":fileName", item.getFileName());
    execQuery();
    query.next();
    return query.value(0).toBool();
}

bool MSTDataSource::deleteMusic(const QStringList& fileList) {
    for (const QString& file : fileList) { // NOLINT(readability-use-anyofallof)
        prepareStatement("DELETE FROM musicInfo WHERE fileName = :fileName");
        bindValue(":fileName", file);
        if (!query.exec()) {
            qWarning() << "[WARN] Error deleting data from database:" << query.lastError().text();
            return false;
        }
    }
    return true;
}
