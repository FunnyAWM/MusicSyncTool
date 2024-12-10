#include "Settings.h"

Settings::Settings(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
	QFile file = QFile("settings.json");
	if (!file.open(QIODevice::ReadOnly)) {
		qDebug() << "[WARN] No settings file found, creating new setting file named settings.json";
		return;
	}
	QJsonDocument settings = QJsonDocument::fromJson(file.readAll());
	if (settings.isNull()) {
		qDebug() << "[WARN] Invalid settings file, creating new setting file named settings.json";
		return;
	}
	QJsonObject obj = settings.object();
	ui.ignoreLyricBox->setChecked(obj["ignoreLyric"].toBool());
	switch (obj["sortBy"].toInt()) {
		case TITLE:
			ui.titleSelect->setChecked(true);
			break;
		case ARTIST:
			ui.artistSelect->setChecked(true);
			break;
		case ALBUM:
			ui.albumSelect->setChecked(true);
			break;
		default:
			ui.titleSelect->setChecked(true);
			break;
	}
	file.close();
}

Settings::set Settings::getSettings() {
	set entity;
	entity.ignoreLyric = ui.ignoreLyricBox->isChecked();
	if (ui.titleSelect->isChecked()) {
		entity.sortBy = TITLE;
	}
	else if (ui.artistSelect->isChecked()) {
		entity.sortBy = ARTIST;
	}
	else if (ui.albumSelect->isChecked()) {
		entity.sortBy = ALBUM;
	}
	return entity;
}

void Settings::on_confirmButton_clicked()
{
	this->close();
	emit confirmPressed(getSettings());
	this->deleteLater();
}

Settings::~Settings()
{}
