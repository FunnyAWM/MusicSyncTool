#ifndef MSTMEDIAPLAYER_H
#define MSTMEDIAPLAYER_H
#include <QAudioOutput>
#include <QMediaPlayer>

class MSTMediaPlayer {
	QMediaPlayer mediaPlayer;
	QAudioOutput audioOutput;
	QString nowPlaying;

public:
	explicit MSTMediaPlayer(QObject* parent);
	
	~MSTMediaPlayer();
	
	[[nodiscard]] bool isPlaying() const;
	
	void play();
	
	void pause();
	
    void stop();
    
	void setNowPlaying(const QString& file);
	
    QMediaPlayer* getMediaPlayer();
    
    [[nodiscard]] qint64 getPosition() const;
    
    [[nodiscard]] qint64 getDuration() const;
    
	void setPosition(qint64 positionMs);
	
	void setVolume(float volumeLevel);
	
    [[nodiscard]] float getVolume() const;
    
    [[nodiscard]] QString getNowPlaying() const;
};

#endif // MSTMEDIAPLAYER_H
