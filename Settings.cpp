#include "Settings.h"

Settings::Settings(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
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
    setIgnoreLyric(obj["ignoreLyric"].toBool());
    setSortBy(obj["sortBy"].toInt());
    setOrderBy(obj["orderBy"].toInt());
    setLanguage(obj["language"].toString());
    setFavoriteTag(obj["favoriteTag"].toString());
    file.close();
}

set Settings::getSettings() {
    set entity;
    entity.ignoreLyric = ui.ignoreLyricBox->isChecked();
    if (ui.titleSelect->isChecked()) {
        entity.sortBy = sortByEnum::TITLE;
    } else if (ui.artistSelect->isChecked()) {
        entity.sortBy = sortByEnum::ARTIST;
    } else if (ui.albumSelect->isChecked()) {
        entity.sortBy = sortByEnum::ALBUM;
    }
    if (ui.chineseButton->isChecked()) {
        entity.language = "Chinese";
    } else if (ui.englishButton->isChecked()) {
        entity.language = "English";
    }
    if (ui.ascButton->isChecked()) {
        entity.orderBy = orderByEnum::ASC;
    } else if (ui.descButton->isChecked()) {
        entity.orderBy = orderByEnum::DESC;
    }
    entity.favoriteTag = ui.favoriteTagEdit->text();
    return entity;
}

void Settings::setIgnoreLyric(bool ignoreLyric) {
    entity.ignoreLyric = ignoreLyric;
    ui.ignoreLyricBox->setChecked(ignoreLyric);
}

void Settings::setSortBy(short sortBy) {
    entity.sortBy = sortBy;
    switch (sortBy) {
    case sortByEnum::TITLE:
        ui.titleSelect->setChecked(true);
        break;
    case sortByEnum::ARTIST:
        ui.artistSelect->setChecked(true);
        break;
    case sortByEnum::ALBUM:
        ui.albumSelect->setChecked(true);
        break;
    default:
        ui.titleSelect->setChecked(true);
        break;
    }
}

void Settings::setLanguage(QString language) {
    entity.language = language;
    if (language == "Chinese") {
        ui.chineseButton->setChecked(true);
    } else if (language == "English") {
        ui.englishButton->setChecked(true);
    } else {
        ui.chineseButton->setChecked(true);
    }
}

void Settings::setFavoriteTag(QString favorite) {
    entity.favoriteTag = favorite;
    ui.favoriteTagEdit->setText(favorite);
}

void Settings::setOrderBy(short orderBy) {
    entity.orderBy = orderBy;
    switch (orderBy) {
    case orderByEnum::ASC:
        ui.ascButton->setChecked(true);
        break;
    case orderByEnum::DESC:
        ui.descButton->setChecked(true);
        break;
    default:
        ui.ascButton->setChecked(true);
        break;
    }
}

void Settings::on_confirmButton_clicked() {
    this->close();
    emit confirmPressed(getSettings());
    this->deleteLater();
}

Settings::~Settings() {}
