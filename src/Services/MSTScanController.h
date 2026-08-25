#ifndef MSTSCANCONTROLLER_H
#define MSTSCANCONTROLLER_H

#include <QDateTime>
#include <QObject>
#include <QString>

#include "MSTTagScanner.h"
#include "../Core/MusicProperties.h"
#include "../Core/SettingEntity.h"
#include "../Data/LyricIgnoreRule.h"

class MSTDataSource;
class MSTTableManager;

using PROPERTIES::LoadErrorType;
using PROPERTIES::AppErrorType;
using PROPERTIES::PathType;

class MSTScanController : public QObject {
	Q_OBJECT

public:
	explicit MSTScanController(MSTDataSource& localDataSource,
	                           MSTDataSource& remoteDataSource,
	                           MSTTableManager* tableManager,
	                           QObject* parent = nullptr);

private:
	MSTDataSource& localDataSource;
	MSTDataSource& remoteDataSource;
	MSTTableManager* tableManager;
	MSTTagScanner tagScanner;
	bool scanned[2] = {false, false};

	[[nodiscard]] MSTDataSource& getDataSource(PathType path) const;

	[[nodiscard]] QString getPath(PathType path) const;

public:
	void startScan(PathType path, unsigned short page, const SettingsData& entity);

	void scanConcurrent(PathType path, unsigned short page, const SettingsData& entity);

	void loadPage(PathType path, unsigned short page, const SettingsData& entity);

	void setFavorite(PathType path, const QStringList& filesToScan, const QString& favoriteTag,
	                 qsizetype progressOffset);

	void setRuleHit(PathType path, const QList<LyricIgnoreRule>& rules,
	                const QStringList& filesToFilter, qsizetype progressOffset);

	[[nodiscard]] MSTTagScanner* getTagScanner();

	void resetScan(PathType path);

signals:
	void scanStart();

	void scanFinished();

	void scanTotal(qsizetype total);

	void scanCurrent(qsizetype current);

	void errorOccurred(AppErrorType type);

	void loadErrorOccurred(QString file, LoadErrorType error);
};

#endif // MSTSCANCONTROLLER_H
