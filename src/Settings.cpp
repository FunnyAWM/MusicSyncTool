#include "Settings.h"

#include <QMessageBox>

#include "AddRuleWidget.h"

Settings::Settings(QWidget* parent) : QWidget(parent) {
    ui.setupUi(this);
    this->ui.rulesWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
    this->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
    this->setWindowModality(Qt::ApplicationModal);
    auto file = QFile("settings.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "[WARN] No settings file found, creating new setting file named settings.json";
        return;
    }
    const QJsonDocument settings = QJsonDocument::fromJson(file.readAll());
    if (settings.isNull()) {
        qWarning() << "[WARN] Invalid settings file, creating new setting file named settings.json";
        return;
    }
    QJsonObject obj = settings.object();
    setIgnoreLyricToUI(obj["ignoreLyric"].toBool());
    setSortByToUI(static_cast<short>(obj["sortBy"].toInt()));
    setOrderByToUI(static_cast<short>(obj["orderBy"].toInt()));
    setLanguageToUI(obj["language"].toString());
    setFavoriteTagToUI(obj["favoriteTag"].toString());
    QList<LyricIgnoreRule> rules;
    QJsonArray rulesArray = obj["rules"].toArray();
    for (QJsonValue rule : rulesArray) {
        QJsonObject ruleObj = rule.toObject();
        rules.append(
            LyricIgnoreRule(LyricIgnoreRule::stringToIgnoreRules(ruleObj["ruleType"].toString()),
                                     LyricIgnoreRule::stringToLyricRules(ruleObj["ruleField"].toString()),
                                     ruleObj["ruleName"].toString()));
    }
    setIgnoreRulesToUI(rules);
    file.close();
}

set Settings::getSettings() {
    set target;
    target.ignoreLyric = ui.ignoreLyricBox->isChecked();
    if (ui.titleSelect->isChecked()) {
        target.sortBy = TITLE;
    } else if (ui.artistSelect->isChecked()) {
        target.sortBy = ARTIST;
    } else if (ui.albumSelect->isChecked()) {
        target.sortBy = ALBUM;
    }
    if (ui.ascButton->isChecked()) {
        target.orderBy = ASC;
    } else if (ui.descButton->isChecked()) {
        target.orderBy = DESC;
    }
    target.language = ui.languageComboBox->currentText();
    target.favoriteTag = ui.favoriteTagEdit->text();
    for (auto& rule : entity.rules) {
        rule.setRulesStr();
    }
    // ReSharper disable once CppSomeObjectMembersMightNotBeInitialized
    target.rules = this->entity.rules;
    return target;
}

void Settings::setIgnoreLyricToUI(const bool ignoreLyric) {
    entity.ignoreLyric = ignoreLyric;
    ui.ignoreLyricBox->setChecked(ignoreLyric);
}

void Settings::setSortByToUI(short sortBy) {
    if (sortBy < 0 || sortBy > 2) {
        sortBy = TITLE;
    }
    entity.sortBy = sortBy;
    switch (sortBy) {
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
}

void Settings::setLanguageToUI(QString language) {
    auto file = QFile(QCoreApplication::applicationDirPath() + "/translations/langinfo.json");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("错误"), tr("无法打开语言文件"));
        qFatal() << "[FATAL] Error opening langinfo.json:" << file.errorString();
    }
    const QJsonDocument langinfo = QJsonDocument::fromJson(file.readAll());
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

void Settings::setFavoriteTagToUI(const QString& favorite) {
    entity.favoriteTag = favorite;
    ui.favoriteTagEdit->setText(favorite);
}

void Settings::setOrderByToUI(short orderBy) {
    if (orderBy < 0 || orderBy > 1) {
        orderBy = ASC;
    }
    entity.orderBy = orderBy;
    switch (orderBy) {
    case ASC:
        ui.ascButton->setChecked(true);
        break;
    case DESC:
        ui.descButton->setChecked(true);
        break;
    default:
        ui.ascButton->setChecked(true);
        break;
    }
}

void Settings::setIgnoreRulesToUI(QList<LyricIgnoreRule>& rules) {
    entity.rules = rules;
    ui.rulesWidget->setRowCount(static_cast<int>(rules.size()));
    int row = 0;
    for (LyricIgnoreRule& singleton : rules) {
        singleton.setRulesStr();
        ui.rulesWidget->setItem(
            row, 0, new QTableWidgetItem(LyricIgnoreRule::lyricRulesToString(singleton.getRuleField())));
        ui.rulesWidget->setItem(
            row, 1, new QTableWidgetItem(LyricIgnoreRule::ignoreRulesToString(singleton.getRuleType())));
        ui.rulesWidget->setItem(row, 2, new QTableWidgetItem(singleton.getRuleName()));
        row++;
    }
}

void Settings::on_confirmButton_clicked() {
    this->close();
    emit confirmPressed(getSettings());
    this->deleteLater();
}

void Settings::on_addRule_clicked() const {
    const auto widget = new AddRuleWidget();
    connect(widget, &AddRuleWidget::sendRules, this, &Settings::addRule);
    widget->show();
}

void Settings::on_deleteSelectedRule_clicked() {
    const auto selectedRows = ui.rulesWidget->selectedItems();
    for (const auto& item : selectedRows) {
        qDebug() << item->row();
        entity.rules.removeAt(item->row());
        ui.rulesWidget->removeRow(item->row());
    }
}

void Settings::addRule(const LyricIgnoreRule& singleton) {
    entity.rules.append(singleton);
    setIgnoreRulesToUI(entity.rules);
}
