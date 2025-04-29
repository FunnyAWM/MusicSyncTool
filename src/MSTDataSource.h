#pragma once

#include <QDateTime>
#include <QSqlQuery>
#include <QString>

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
    [[nodiscard]] QString getPath() const { return path; }
    [[nodiscard]] bool isOpen() const { return db.isOpen(); }
    void setConnectionName(const QString&);
    [[nodiscard]] bool openDB(const QString& path_);
    [[nodiscard]] bool openDB();
    void initTable();
    void closeDB();
    void prepareStatement(const QString&);
    template <class T>
    void bindValue(const QString&, const T&);
    void execQuery();
    void setFavorite(const QString&, const QDateTime&);
    void setRuleHit(const QList<LyricIgnoreRule>&, const QDateTime&);
    QList<QueryItem> getAll(const QVector<QueryRows>&);
    QStringList addMusic(const QStringList&);
    [[nodiscard]] bool addMusic(const QString&);
    QList<QueryItem> searchMusic(const QString&);
    QStringList getFileNameByMD(const QList<QueryItem>&);
    QList<QueryItem> getFavorite(unsigned short, SortByEnum, OrderByEnum);
    QList<QueryItem> getRuleHit(unsigned short, SortByEnum, OrderByEnum);
    QList<QueryItem> getMusicToTable(unsigned short, SortByEnum, OrderByEnum);
    int getCount();
    [[nodiscard]] bool deleteMusic(const QStringList&);
signals:
    void loadStarted();
    void loadFinished();
    void loadFinished(OperationType);
    void totalSize(qsizetype size);
    void currentProgress(qsizetype progress);
};
