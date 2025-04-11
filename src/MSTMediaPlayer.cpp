#include "MSTMediaPlayer.h"

MSTMediaPlayer::MSTMediaPlayer(QObject* parent) {
    mediaPlayer.setParent(parent);
    audioOutput.setParent(parent);
    mediaPlayer.setAudioOutput(&audioOutput);
    mediaPlayer.setPlaybackRate(1.0);
}

MSTMediaPlayer::~MSTMediaPlayer() { mediaPlayer.stop(); }

QMediaPlayer* MSTMediaPlayer::getMediaPlayer() { return &mediaPlayer; }
qint64 MSTMediaPlayer::getPosition() const { return mediaPlayer.position(); }
qint64 MSTMediaPlayer::getDuration() const { return mediaPlayer.duration(); }

void MSTMediaPlayer::setNowPlaying(const QString& file) {
    nowPlaying = file;
    mediaPlayer.setSource(QUrl::fromLocalFile(file));
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

void MSTMediaPlayer::stop() {
    if (!isPlaying() || mediaPlayer.mediaStatus() == QMediaPlayer::NoMedia) {
        return;
    }
    mediaPlayer.stop();
}

void MSTMediaPlayer::setPosition(const qint64 pos) { mediaPlayer.setPosition(pos); }

void MSTMediaPlayer::setVolume(const float vol) { audioOutput.setVolume(vol); }

float MSTMediaPlayer::getVolume() const { return audioOutput.volume(); }

QString MSTMediaPlayer::getNowPlaying() const { return nowPlaying; }

bool MSTMediaPlayer::isPlaying() const { return mediaPlayer.playbackState() == QMediaPlayer::PlayingState; }
