#include "Settings.h"

Settings::Settings(QWidget *parent) : QWidget(parent) {
    ui.setupUi(this);

    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
    this->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
    this->setWindowModality(Qt::ApplicationModal);
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
    setIgnoreLyricToUI(obj["ignoreLyric"].toBool());
    setSortByToUI(obj["sortBy"].toInt());
    setOrderByToUI(obj["orderBy"].toInt());
    setLanguageToUI(obj["language"].toString());
    setFavoriteTagToUI(obj["favoriteTag"].toString());
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
    if (ui.ascButton->isChecked()) {
        entity.orderBy = orderByEnum::ASC;
    } else if (ui.descButton->isChecked()) {
        entity.orderBy = orderByEnum::DESC;
    }
    entity.language = ui.languageComboBox->currentText();
    entity.favoriteTag = ui.favoriteTagEdit->text();
    return entity;
}

void Settings::setIgnoreLyricToUI(bool ignoreLyric) {
    entity.ignoreLyric = ignoreLyric;
    ui.ignoreLyricBox->setChecked(ignoreLyric);
}

void Settings::setSortByToUI(short sortBy) {
    if (sortBy < 0 || sortBy > 2) {
        sortBy = sortByEnum::TITLE;
    }
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

void Settings::setLanguageToUI(QString language) {
    QFile file = QFile(QCoreApplication::applicationDirPath() + "/translations/langinfo.json");
    assert(file.open(QIODevice::ReadOnly));
    if (!file.isOpen()) {
        file.open(QIODevice::ReadOnly);
    }
    QJsonDocument langinfo = QJsonDocument::fromJson(file.readAll());
    QJsonArray langArray = langinfo.array();
    for (QJsonValue lang : langArray) {
        QJsonObject langObj = lang.toObject();
        ui.languageComboBox->addItem(langObj["lang"].toString());
    }
    if (language.isEmpty()) {
        if (ui.languageComboBox->findText(QString::fromUtf8("中文")) != -1) {
            language = QString::fromUtf8("中文");
        } else {
            language = ui.languageComboBox->itemText(0);
            ui.languageComboBox->setCurrentText(language);
        }
    }
    ui.languageComboBox->setCurrentText(language);
    entity.language = language;
}

void Settings::setFavoriteTagToUI(QString favorite) {
    entity.favoriteTag = favorite;
    ui.favoriteTagEdit->setText(favorite);
}

void Settings::setOrderByToUI(short orderBy) {
    if (orderBy < 0 || orderBy > 1) {
        orderBy = orderByEnum::ASC;
    }
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
