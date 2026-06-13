/**
 * @file MSTMediaController.h
 * @brief 媒体播放控制器类定义
 * @details 封装媒体播放器的UI控制逻辑，包括播放/暂停、进度滑块、
 *          音量控制和曲目选择，将播放控制职责从主窗口中分离出来
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

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
	MSTMediaPlayer* player;            ///< 媒体播放器指针
	QPushButton* playControl;          ///< 播放/暂停按钮
	QSlider* playSlider;               ///< 播放进度滑块
	QLabel* playProgress;              ///< 播放进度标签
	QSlider* volumeSlider;             ///< 音量滑块
	QLabel* volumeLabel;               ///< 音量标签
	QLabel* nowPlayingLabel;           ///< 正在播放标签
	QTableWidget* tableLocal;          ///< 本地音乐表格
	QTableWidget* tableRemote;         ///< 远程音乐表格
	MSTDataSource& localDataSource;    ///< 本地数据源引用
	MSTDataSource& remoteDataSource;   ///< 远程数据源引用
	QString nowPlaying;                ///< 当前播放的文件名

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
