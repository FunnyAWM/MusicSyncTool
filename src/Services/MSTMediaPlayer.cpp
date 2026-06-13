/**
 * @file MSTMediaPlayer.cpp
 * @brief 音乐同步工具媒体播放器类的实现
 * @details 实现媒体播放器的构造、播放控制、音量和进度管理功能
 * @author FunnyAWM
 * @version 2.3.0
 * @date 2024
 */

#include "MSTMediaPlayer.h"

/**
 * @brief 构造函数，初始化媒体播放器
 * @param parent 父对象指针
 */
MSTMediaPlayer::MSTMediaPlayer(QObject* parent) {
    mediaPlayer.setParent(parent);              // 设置媒体播放器的父对象
    audioOutput.setParent(parent);              // 设置音频输出的父对象
    mediaPlayer.setAudioOutput(&audioOutput);  // 将音频输出连接到媒体播放器
    mediaPlayer.setPlaybackRate(1.0);          // 设置播放速率为正常速度
}

/**
 * @brief 析构函数，停止播放并清理资源
 */
MSTMediaPlayer::~MSTMediaPlayer() { 
    mediaPlayer.stop(); 
}

/**
 * @brief 检查是否正在播放
 * @return 如果正在播放返回true，否则返回false
 */
bool MSTMediaPlayer::isPlaying() const { 
    return mediaPlayer.playbackState() == QMediaPlayer::PlayingState; 
}

/**
 * @brief 播放音乐
 * 如果已经在播放或没有媒体文件则不执行任何操作
 */
void MSTMediaPlayer::play() {
    if (isPlaying() || mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia) {
        return;
    }
    mediaPlayer.play();
}

/**
 * @brief 暂停播放
 * 如果没有在播放或没有媒体文件则不执行任何操作
 */
void MSTMediaPlayer::pause() {
    if (!isPlaying() || mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia) {
        return;
    }
    mediaPlayer.pause();
}

/**
 * @brief 停止播放
 * 如果没有在播放或没有媒体文件则不执行任何操作
 */
void MSTMediaPlayer::stop() {
    if (!isPlaying() || mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia) {
        return;
    }
    mediaPlayer.stop();
}

/**
 * @brief 设置当前播放的音乐文件
 * @param file 音乐文件的完整路径
 */
void MSTMediaPlayer::setNowPlaying(const QString& file) {
    nowPlaying = file.split("/").last();               // 提取文件名（去除路径）
    mediaPlayer.setSource(QUrl::fromLocalFile(file)); // 设置媒体播放器的音频源
}

/**
 * @brief 获取媒体播放器对象指针
 * @return QMediaPlayer对象指针
 */
QMediaPlayer* MSTMediaPlayer::getMediaPlayer() { 
    return &mediaPlayer; 
}

/**
 * @brief 获取当前播放位置
 * @return 当前播放位置（毫秒）
 */
qint64 MSTMediaPlayer::getPosition() const { 
    return mediaPlayer.position(); 
}

/**
 * @brief 获取音乐总时长
 * @return 音乐总时长（毫秒）
 */
qint64 MSTMediaPlayer::getDuration() const { 
    return mediaPlayer.duration(); 
}

/**
 * @brief 设置播放位置
 * @param pos 目标播放位置（毫秒）
 */
void MSTMediaPlayer::setPosition(const qint64 positionMs) { 
    mediaPlayer.setPosition(positionMs); 
}

/**
 * @brief 设置音量
 * @param vol 音量值（0.0-1.0范围）
 */
void MSTMediaPlayer::setVolume(const float volumeLevel) { 
    audioOutput.setVolume(volumeLevel); 
}

/**
 * @brief 获取当前音量
 * @return 当前音量值（0.0-1.0范围）
 */
float MSTMediaPlayer::getVolume() const { 
    return audioOutput.volume(); 
}

/**
 * @brief 获取当前播放的音乐文件名
 * @return 当前播放的音乐文件名（不包含路径）
 */
QString MSTMediaPlayer::getNowPlaying() const { 
    return nowPlaying; 
}
