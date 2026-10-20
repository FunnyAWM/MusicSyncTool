#ifndef MEDIAMEDIACONTROLLER_H
#define MEDIAMEDIACONTROLLER_H

#include <QMediaPlayer>
#include <QObject>
#include <QString>

#include "../../Core/MusicProperties.h"

class QLabel;
class QSlider;
class QPushButton;
class QTableWidget;
class MSTMediaPlayer;
class MSTDataSource;

using PROPERTIES::PathType;
using PROPERTIES::AppErrorType;
using PROPERTIES::PlayState;

/**
 * @brief 媒体播放控制器
 * @details 负责媒体播放相关的UI控制逻辑，包括：
 *          - 播放/暂停切换与UI状态同步
 *          - 播放进度滑块控制与时间格式化
 *          - 音量滑块控制与标签更新
 *          - 双击表格行选择曲目播放
 *          - 播放结束状态处理
 */
class MSTMediaController : public QObject {
	Q_OBJECT

public:
	explicit MSTMediaController(
		MSTMediaPlayer* player,
		QPushButton* playControl,
		QSlider* playSlider,
		QLabel* playProgress,
		QSlider* volumeSlider,
		QLabel* volumeLabel,
		QLabel* nowPlayingLabel,
		QTableWidget* tableLocal,
		QTableWidget* tableRemote,
		MSTDataSource& localDataSource,
		MSTDataSource& remoteDataSource,
		QObject* parent = nullptr);

private:
	MSTMediaPlayer* player;
	QPushButton* playControl;
	QSlider* playSlider;
	QLabel* playProgress;
	QSlider* volumeSlider;
	QLabel* volumeLabel;
	QLabel* nowPlayingLabel;
	QTableWidget* tableLocal;
	QTableWidget* tableRemote;
	MSTDataSource& localDataSource;
	MSTDataSource& remoteDataSource;
	QString nowPlaying;

	[[nodiscard]] static QString formatTime(qint64 ms);

public:
	void initMediaPlayer() const;

	void connectSignals();

	void togglePlayPause();

	void playSelectedTrack(PathType path, int row);

	void setMediaWidget(PlayState state) const;

	void setNowPlayingTitle(const QString& file) const;

	void seekToPosition(int position) const;

	void updateSliderPosition(qint64 position) const;

	void onPlaySliderPressed() const;

	void onVolumeSliderPressed() const;

	void setVolumeFromSlider(int position) const;

	void handleMediaEnd(QMediaPlayer::PlaybackState state) const;

signals:
	/**
	 * @brief 错误信号
	 * @param type 错误类型
	 */
	void errorOccurred(AppErrorType type);
};

#endif // MEDIAMEDIACONTROLLER_H
