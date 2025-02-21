#pragma once
#include <QAudioOutput>
#include <QMediaPlayer>

class MSTMediaPlayer {
private:
	QMediaPlayer mediaPlayer;
	QAudioOutput audioOutput;
public:
	MSTMediaPlayer();
	~MSTMediaPlayer();
	void setFile(const QString&);
	void play();
	void pause();
	void stop();
	void setPosition(qint64);
	void setVolume(double);
	bool isPlaying();
};
