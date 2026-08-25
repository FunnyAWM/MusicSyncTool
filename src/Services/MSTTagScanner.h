#ifndef MSTTAGSCANNER_H
#define MSTTAGSCANNER_H

#include <QList>
#include <QObject>
#include <QString>

#include "../Data/LyricIgnoreRule.h"
#include "../Data/QueryItem.h"

struct FavoriteUpdate {
	QString fileName;
	bool isFavorite;
};

struct RuleHitUpdate {
	QString fileName;
	bool isRuleHit;
};

class MSTTagScanner : public QObject {
	Q_OBJECT

public:
	explicit MSTTagScanner(QObject* parent = nullptr);
	~MSTTagScanner() override = default;

	QList<FavoriteUpdate> scanFavorite(const QString& basePath,
	                                   const QStringList& fileNames,
	                                   const QString& tag);

	QList<RuleHitUpdate> scanRuleHit(const QString& basePath,
	                                 const QList<QueryItem>& items,
	                                 const QList<LyricIgnoreRule>& rules);

signals:
	void loadStarted();

	void totalSize(qsizetype size);

	void currentProgress(qsizetype progress);

	void loadFinished();
};

#endif // MSTTAGSCANNER_H
