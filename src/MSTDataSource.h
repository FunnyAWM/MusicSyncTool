#pragma once

// ReSharper disable once CppUnusedIncludeDirective
#include <QRegularExpression>
// ReSharper disable once CppUnusedIncludeDirective
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QString>
#include <QDateTime>

#include "LyricIgnoreRule.h"
#include "QueryItem.h"

class MSTDataSource final : public QObject {

    Q_OBJECT

    QString path;
    QString connection;
    QSqlDatabase db;
    QSqlQuery query;
    int pageSize = 200;

public:
    [[nodiscard]] int getPageSize() const;
    void setPageSize(int page_size);
    explicit MSTDataSource(const QString&);
    explicit MSTDataSource() = default;
    ~MSTDataSource() override = default;
    void setPath(const QString&);
	QString getPath() const { return path; }
	bool isOpen() const { return db.isOpen(); }
    void setConnectionName(const QString&);
    [[nodiscard]] bool openDB(const QString& path_);
    [[nodiscard]] bool openDB();
    void initTable();
    void closeDB();
    void prepareStatement(const QString&);
    template <class T>
    void bindValue(const QString&, const T&);
    void execQuery();
    void setFavorite(const QString&, QDateTime);
    void setRuleHit(const QList<LyricIgnoreRule>&, QDateTime);
    QList<QueryItem> getAll(const QVector<QueryRows>&);
    QStringList addMusic(const QStringList&);
    [[nodiscard]] bool addMusic(const QString&);
    QList<QueryItem> searchMusic(const QString&);
    QStringList getFileNameByMD(const QList<QueryItem>&);
    bool getFavorite(const QueryItem&);
    bool getRuleHit(const QueryItem&);
    QList<QueryItem> getMusicToTable(short, SortByEnum, OrderByEnum);
    int getCount();
    [[nodiscard]] bool deleteMusic(const QStringList&);
};

