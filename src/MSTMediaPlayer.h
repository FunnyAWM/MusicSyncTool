#pragma once
#include <QAudioOutput>
#include <QMediaPlayer>

class MSTMediaPlayer {
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
    QString getNowPlaying() const;
	[[nodiscard]] bool isPlaying() const;
};
