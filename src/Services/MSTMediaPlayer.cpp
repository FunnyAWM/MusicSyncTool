#include "MSTMediaPlayer.h"

#include <QFileInfo>

MSTMediaPlayer::MSTMediaPlayer(QObject* parent) {
    mediaPlayer.setParent(parent);
    audioOutput.setParent(parent);
    mediaPlayer.setAudioOutput(&audioOutput);
    mediaPlayer.setPlaybackRate(1.0);
}

MSTMediaPlayer::~MSTMediaPlayer() { 
    mediaPlayer.stop(); 
}

bool MSTMediaPlayer::isPlaying() const { 
    return mediaPlayer.playbackState() == QMediaPlayer::PlayingState; 
}

void MSTMediaPlayer::play() {
    if (isPlaying() || mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia) {
        return;
    }
    mediaPlayer.play();
}

void MSTMediaPlayer::pause() {
    if (!isPlaying() || mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia) {
        return;
    }
    mediaPlayer.pause();
}

void MSTMediaPlayer::setNowPlaying(const QString& file) {
    // 用 QFileInfo 取文件名，避免手写分隔符在非 Windows 平台失效
    nowPlaying = QFileInfo(file).fileName();
    mediaPlayer.setSource(QUrl::fromLocalFile(file));
}

QMediaPlayer* MSTMediaPlayer::getMediaPlayer() { 
    return &mediaPlayer; 
}

qint64 MSTMediaPlayer::getPosition() const { 
    return mediaPlayer.position(); 
}

qint64 MSTMediaPlayer::getDuration() const { 
    return mediaPlayer.duration(); 
}

void MSTMediaPlayer::setPosition(const qint64 positionMs) { 
    mediaPlayer.setPosition(positionMs); 
}

void MSTMediaPlayer::setVolume(const float volumeLevel) { 
    audioOutput.setVolume(volumeLevel); 
}

float MSTMediaPlayer::getVolume() const { 
    return audioOutput.volume(); 
}

QString MSTMediaPlayer::getNowPlaying() const { 
    return nowPlaying; 
}
