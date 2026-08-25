#include "MSTMediaController.h"

#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <taglib/fileref.h>

#include "../../Core/MSTTagUtils.h"
#include "../../Data/QueryItem.h"
#include "../../Services/MSTDataSource.h"
#include "../../Services/MSTMediaPlayer.h"

MSTMediaController::MSTMediaController(
	MSTMediaPlayer* player,
	QPushButton* playControl,
	QSlider* playSlider,
	QLabel* playProgress,
	QSlider* volumeSlider,
	QLabel* volumeLabel,
	QLabel* nowPlayingLabel,
	QTableWidget* tableLocal,
	QTableWidget* tableRemote,
	MSTDataSource& localDataSource,
	MSTDataSource& remoteDataSource,
	QObject* parent)
	: QObject(parent),
	  player(player),
	  playControl(playControl),
	  playSlider(playSlider),
	  playProgress(playProgress),
	  volumeSlider(volumeSlider),
	  volumeLabel(volumeLabel),
	  nowPlayingLabel(nowPlayingLabel),
	  tableLocal(tableLocal),
	  tableRemote(tableRemote),
	  localDataSource(localDataSource),
	  remoteDataSource(remoteDataSource) {
}

QString MSTMediaController::formatTime(const qint64 ms) {
	const qint64 minutes = ms / 60000;
	const qint64 seconds = ms % 60000 / 1000;
	if (seconds < 10) {
		return QString::number(minutes) + ":0" + QString::number(seconds);
	}
	return QString::number(minutes) + ":" + QString::number(seconds);
}

void MSTMediaController::initMediaPlayer() const {
	player->setVolume(0.5);
}

void MSTMediaController::connectSignals() {
	connect(player->getMediaPlayer(), &QMediaPlayer::positionChanged,
	        this, &MSTMediaController::updateSliderPosition);
	connect(player->getMediaPlayer(), &QMediaPlayer::playbackStateChanged,
	        this, &MSTMediaController::handleMediaEnd);
}

void MSTMediaController::togglePlayPause() {
	if (player->getNowPlaying().isEmpty()) {
		emit errorOccurred(AppErrorType::NO_AUDIO);
		return;
	}
	if (player->isPlaying()) {
		player->pause();
		setMediaWidget(PlayState::PAUSED);
	}
	else {
		player->play();
		setMediaWidget(PlayState::PLAYING);
	}
}

void MSTMediaController::playSelectedTrack(const PathType path, const int row) {
	const QTableWidget& tableWidget = path == PathType::LOCAL ? *tableLocal : *tableRemote;
	MSTDataSource& dataSource = path == PathType::LOCAL ? localDataSource : remoteDataSource;

	// 行号越界或单元格为空时中止
	if (row < 0 || row >= tableWidget.rowCount()) {
		emit errorOccurred(AppErrorType::NO_FILE);
		return;
	}
	if (!tableWidget.item(row, 0) || !tableWidget.item(row, 1) || !tableWidget.item(row, 2)) {
		emit errorOccurred(AppErrorType::NO_FILE);
		return;
	}

	const auto item = std::make_shared<QueryItem>();
	item->setTitle(tableWidget.item(row, 0)->text());
	item->setArtist(tableWidget.item(row, 1)->text());
	item->setAlbum(tableWidget.item(row, 2)->text());
	QList<QueryItem> file;
	file.append(*item);
	const QStringList fileNames = dataSource.getFileNameByMetadata(file);
	if (fileNames.isEmpty()) {
		emit errorOccurred(AppErrorType::NO_FILE);
		return;
	}
	nowPlaying = fileNames.at(0);

	const QString filePath = (path == PathType::LOCAL
		                          ? localDataSource.getPath()
		                          : remoteDataSource.getPath()) + "/" + nowPlaying;
	player->setNowPlaying(filePath);

	const TagLib::FileRef fileRef = MSTTagUtils::createFileRef(filePath);

	if (!fileRef.isNull() && fileRef.audioProperties()) {
		const qint64 length = fileRef.audioProperties()->lengthInMilliseconds();
		setNowPlayingTitle(nowPlaying);
		player->setPosition(0);
		player->play();
		setMediaWidget(PlayState::PLAYING);
		playSlider->setMaximum(static_cast<int>(length));
		playSlider->setValue(0);
		playProgress->setText("00:00");
	}
}

void MSTMediaController::setMediaWidget(const PlayState state) const {
	if (state == PlayState::PLAYING) {
		playControl->setText(tr("暂停"));
		setNowPlayingTitle(player->getNowPlaying());
	}
	else if (state == PlayState::PAUSED) {
		playControl->setText(tr("播放"));
	}
	else if (state == PlayState::STOPPED) {
		playControl->setText(tr("播放"));
		nowPlayingLabel->setText(tr("播放已结束。"));
	}
	volumeLabel->setText(tr("音量：") + QString::number(volumeSlider->value()) + "%");
}

void MSTMediaController::setNowPlayingTitle(const QString& file) const {
	nowPlayingLabel->setText(tr("正在播放：") + file);
}

void MSTMediaController::seekToPosition(const int position) const {
	player->setPosition(position);
	playProgress->setText(formatTime(position));
}

void MSTMediaController::updateSliderPosition(const qint64 position) const {
	playSlider->setValue(static_cast<int>(position));
	playProgress->setText(formatTime(position) + "/" + formatTime(player->getDuration()));
}

void MSTMediaController::onPlaySliderPressed() const {
	player->setPosition(playSlider->value());
}

void MSTMediaController::onVolumeSliderPressed() const {
	player->setVolume(static_cast<float>(volumeSlider->value() / 100.0));
	volumeLabel->setText(tr("音量：") + QString::number(volumeSlider->value()) + "%");
}

void MSTMediaController::setVolumeFromSlider(const int position) const {
	player->setVolume(static_cast<float>(position / 100.0));
	volumeLabel->setText(tr("音量：") + QString::number(position) + "%");
}

void MSTMediaController::handleMediaEnd(const QMediaPlayer::PlaybackState state) const {
	if (state == QMediaPlayer::PlaybackState::StoppedState) {
		playControl->setText(tr("播放"));
		nowPlayingLabel->setText(tr("播放已结束。"));
		playSlider->setValue(0);
		playProgress->setText("00:00");
	}
}
