#pragma once
#include <QAudioOutput>
#include <QMediaPlayer>

class MSTMediaPlayer {
private:
	QMediaPlayer mediaPlayer;
	QAudioOutput audioOutput;
	QString nowPlaying;
public:
	MSTMediaPlayer();
	~MSTMediaPlayer();
	void setFile(const QString&);
	void play();
	void pause();
	void setPosition(qint64);
	void setVolume(float);
	[[nodiscard]] bool isPlaying() const;
};
