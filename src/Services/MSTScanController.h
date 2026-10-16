#ifndef MSTSCANCONTROLLER_H
#define MSTSCANCONTROLLER_H

#include <atomic>
#include <QDateTime>
#include <QFutureWatcher>
#include <QObject>
#include <QString>

#include "MSTTagScanner.h"
#include "../Core/MusicProperties.h"
#include "../Core/SettingEntity.h"
#include "../Data/LyricIgnoreRule.h"
#include "../Data/QueryItem.h"

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
	/// @brief 已扫描标记：工作线程写入、GUI 线程读取/复位，故用原子量
	std::atomic<bool> scanned[2] = {false, false};
	/// @brief 持有扫描任务的 QFuture，保证窗口析构前任务已结束
	QFutureWatcher<void> scanWatcher;

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

	/// @brief 是否存在尚未结束的扫描任务
	[[nodiscard]] bool isScanRunning() const;

	/// @brief 等待扫描任务结束（析构前必须调用，避免任务访问已销毁对象）
	void waitForFinished();

signals:
	void scanStart();

	void scanFinished();

	/// @brief 扫描总数；跨线程排队投递，参数用 int 保证元类型在 moc 侧可解析
	void scanTotal(int total);

	void scanCurrent(int current);

	void errorOccurred(AppErrorType type);

	void loadErrorOccurred(QString file, LoadErrorType error);

	/**
	 * @brief 请求 GUI 线程刷新表格
	 * @param path 路径类型
	 * @param items 当前页数据
	 * @param page 当前页码
	 * @param totalPages 总页数
	 * @details 工作线程禁止直接操作 QTableWidget/QLabel，表格刷新统一回到 GUI 线程
	 */
	void tablePageReady(PathType path, QList<QueryItem> items, unsigned short page, unsigned short totalPages);
};

#endif // MSTSCANCONTROLLER_H
