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