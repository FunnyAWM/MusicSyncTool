#include "MSTMediaPlayer.h"

MSTMediaPlayer::MSTMediaPlayer() {
	mediaPlayer.setAudioOutput(&audioOutput);
}

MSTMediaPlayer::~MSTMediaPlayer() {
	mediaPlayer.stop();
}

void MSTMediaPlayer::setFile(const QString& file) {
	mediaPlayer.setSource(QUrl::fromLocalFile(file));
}

void MSTMediaPlayer::play()
{
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

void MSTMediaPlayer::setPosition(const qint64 pos) {
	mediaPlayer.setPosition(pos);
}

void MSTMediaPlayer::setVolume(const float vol) { audioOutput.setVolume(vol); }
QString MSTMediaPlayer::getNowPlaying() const {
    return nowPlaying;
}

bool MSTMediaPlayer::isPlaying() const {
	return mediaPlayer.playbackState() == QMediaPlayer::PlayingState;
}
