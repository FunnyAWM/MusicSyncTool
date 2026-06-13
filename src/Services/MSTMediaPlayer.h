/**
 * @file MSTMediaPlayer.h
 * @brief 音乐同步工具媒体播放器类定义
 * @details 封装了QMediaPlayer和QAudioOutput，提供音乐播放的基本功能，
 *          包括播放、暂停、停止、音量控制和进度管理
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#ifndef MSTMEDIAPLAYER_H
#define MSTMEDIAPLAYER_H
#include <QAudioOutput>
#include <QMediaPlayer>

/**
 * @brief 音乐同步工具媒体播放器类
 * 封装了QMediaPlayer和QAudioOutput，提供统一的音乐播放接口
 */
class MSTMediaPlayer {
	QMediaPlayer mediaPlayer;  ///< 媒体播放器对象
	QAudioOutput audioOutput;  ///< 音频输出对象
	QString nowPlaying;        ///< 当前播放的音乐文件名

public:
	explicit MSTMediaPlayer(QObject* parent);
	
	~MSTMediaPlayer();
	
	[[nodiscard]] bool isPlaying() const;
	
	void play();
	
	void pause();
	
    void stop();
    
	void setNowPlaying(const QString& file);
	
    QMediaPlayer* getMediaPlayer();
    
    [[nodiscard]] qint64 getPosition() const;
    
    [[nodiscard]] qint64 getDuration() const;
    
	void setPosition(qint64 positionMs);
	
	void setVolume(float volumeLevel);
	
    [[nodiscard]] float getVolume() const;
    
    [[nodiscard]] QString getNowPlaying() const;
};

#endif // MSTMEDIAPLAYER_H
