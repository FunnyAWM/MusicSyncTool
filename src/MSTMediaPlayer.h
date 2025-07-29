#pragma once
#include <QAudioOutput>
#include <QMediaPlayer>

/**
 * @brief 音乐同步工具媒体播放器类
 * 封装了QMediaPlayer和QAudioOutput，提供音乐播放的基本功能
 */
class MSTMediaPlayer {
	QMediaPlayer mediaPlayer;  // 媒体播放器对象
	QAudioOutput audioOutput;  // 音频输出对象
	QString nowPlaying;        // 当前播放的音乐文件名

public:
	/**
	 * @brief 构造函数
	 * @param parent 父对象指针
	 */
	explicit MSTMediaPlayer(QObject* parent);
	
	/**
	 * @brief 析构函数
	 */
	~MSTMediaPlayer();
	
	/**
	 * @brief 设置当前播放的音乐文件
	 * @param file 音乐文件路径
	 */
	void setNowPlaying(const QString& file);
	
	/**
	 * @brief 获取媒体播放器对象指针
	 * @return QMediaPlayer对象指针
	 */
    QMediaPlayer* getMediaPlayer();
    
    /**
     * @brief 获取当前播放位置
     * @return 当前播放位置（毫秒）
     */
    [[nodiscard]] qint64 getPosition() const;
    
    /**
     * @brief 获取音乐总时长
     * @return 音乐总时长（毫秒）
     */
    [[nodiscard]] qint64 getDuration() const;
    
	/**
	 * @brief 播放音乐
	 */
	void play();
	
	/**
	 * @brief 暂停播放
	 */
	void pause();
	
	/**
	 * @brief 停止播放
	 */
    void stop();
    
	/**
	 * @brief 设置播放位置
	 * @param pos 目标播放位置（毫秒）
	 */
	void setPosition(qint64 pos);
	
	/**
	 * @brief 设置音量
	 * @param vol 音量值（0.0-1.0）
	 */
	void setVolume(float vol);
	
	/**
	 * @brief 获取当前音量
	 * @return 当前音量值（0.0-1.0）
	 */
    [[nodiscard]] float getVolume() const;
    
    /**
     * @brief 获取当前播放的音乐文件名
     * @return 当前播放的音乐文件名
     */
    [[nodiscard]] QString getNowPlaying() const;
    
	/**
	 * @brief 检查是否正在播放
	 * @return 如果正在播放返回true，否则返回false
	 */
	[[nodiscard]] bool isPlaying() const;
};
