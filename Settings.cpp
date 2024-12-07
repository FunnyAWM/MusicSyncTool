#include "Settings.h"

Settings::Settings(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	this->setWindowFlags(Qt::WindowCloseButtonHint);
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
	file.close();
}

Settings::set Settings::getSettings() {
	return set{ ui.ignoreLyricBox->isChecked() };
}

void Settings::on_confirmButton_clicked()
{
	emit confirmPressed(getSettings());
	this->close();
}

Settings::~Settings()
{}
