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
using PROPERTIES::PET;
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
	/**
	 * @brief 构造函数
	 * @param player 媒体播放器共享指针
	 * @param playControl 播放/暂停按钮
	 * @param playSlider 播放进度滑块
	 * @param playProgress 播放进度标签
	 * @param volumeSlider 音量滑块
	 * @param volumeLabel 音量标签
	 * @param nowPlayingLabel 正在播放标签
	 * @param tableLocal 本地音乐表格
	 * @param tableRemote 远程音乐表格
	 * @param localDataSource 本地数据源引用
	 * @param remoteDataSource 远程数据源引用
	 * @param parent 父对象指针
	 */
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

	/**
	 * @brief 初始化媒体播放器
	 * @details 设置初始音量为50%
	 */
	void initMediaPlayer() const;

	/**
	 * @brief 根据播放状态更新UI控件
	 * @param state 播放状态枚举
	 */
	void setMediaWidget(PlayState state) const;

	/**
	 * @brief 设置正在播放的标题显示
	 * @param file 文件名
	 */
	void setNowPlayingTitle(const QString& file) const;

	/**
	 * @brief 播放或暂停切换
	 * @details 根据当前播放状态切换播放/暂停，并更新UI
	 */
	void togglePlayPause();

	/**
	 * @brief 拖动播放进度滑块
	 * @param position 滑块位置（毫秒）
	 */
	void seekToPosition(int position) const;

	/**
	 * @brief 按下播放进度滑块
	 */
	void onPlaySliderPressed() const;

	/**
	 * @brief 设置音量（从滑块值）
	 * @param position 滑块位置（0-100）
	 */
	void setVolumeFromSlider(int position) const;

	/**
	 * @brief 按下音量滑块
	 */
	void onVolumeSliderPressed() const;

	/**
	 * @brief 更新播放进度显示
	 * @param position 当前播放位置（毫秒）
	 */
	void updateSliderPosition(qint64 position) const;

	/**
	 * @brief 处理播放结束状态
	 * @param state 播放器状态
	 */
	void handleMediaEnd(QMediaPlayer::PlaybackState state) const;

	/**
	 * @brief 播放选定的曲目
	 * @param path 路径类型（本地或远程）
	 * @param row 表格行号
	 */
	void playSelectedTrack(PathType path, int row);

	/**
	 * @brief 连接媒体播放器信号
	 * @details 将播放器内部信号连接到控制器的处理方法
	 */
	void connectSignals();

signals:
	/**
	 * @brief 错误信号
	 * @param type 错误类型
	 */
	void errorOccurred(PET type);

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

	/**
	 * @brief 格式化毫秒时间为 mm:ss 字符串
	 * @param ms 毫秒数
	 * @return 格式化后的时间字符串
	 */
	[[nodiscard]] static QString formatTime(qint64 ms);
};

#endif // MEDIAMEDIACONTROLLER_H
