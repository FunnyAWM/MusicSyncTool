#pragma once
#include <QAudioOutput>
#include <QMediaPlayer>

class MSTMediaPlayer {
	QMediaPlayer mediaPlayer;
	QAudioOutput audioOutput;
	QString nowPlaying;
public:
	explicit MSTMediaPlayer(QObject*);
	~MSTMediaPlayer();
	void setNowPlaying(const QString&);
    QMediaPlayer* getMediaPlayer();
    [[nodiscard]] qint64 getPosition() const;
    [[nodiscard]] qint64 getDuration() const;
	void play();
	void pause();
    void stop();
	void setPosition(qint64);
	void setVolume(float);
    [[nodiscard]] float getVolume() const;
    [[nodiscard]] QString getNowPlaying() const;
	[[nodiscard]] bool isPlaying() const;
};
