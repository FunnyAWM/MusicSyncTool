#include "Settings.h"
#include "../../Logger.h"
#include <QMessageBox>

#include "../AddRuleWidget/AddRuleWidget.h"

/**
 * @brief 设置界面构造函数
 * 初始化UI并加载当前设置
 * @param parent 父窗口指针
 */
Settings::Settings(QWidget* parent) : QWidget(parent) {
    ui.setupUi(this);                                                                           // 设置UI界面
    this->ui.rulesWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);      // 设置表格头部自动拉伸
    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));                                         // 设置窗口图标
    this->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);         // 设置窗口标志
    this->setWindowModality(Qt::ApplicationModal);                                             // 设置为应用程序模态对话框
    
    // 读取设置文件
    auto file = QFile("settings.json");
    if (!file.open(QIODevice::ReadOnly)) {
        Logger::Warn("No settings file found, creating new setting file named settings.json");
        return;
    }
    
    // 解析JSON设置文件
    const QJsonDocument settings = QJsonDocument::fromJson(file.readAll());
    if (settings.isNull()) {
        Logger::Warn("Invalid settings file, creating new setting file named settings.json");
        return;
    }
    
    QJsonObject obj = settings.object();
    
    // 将设置加载到UI控件
    setIgnoreLyricToUI(obj["ignoreLyric"].toBool());
    setSortByToUI(static_cast<short>(obj["sortBy"].toInt()));
    setOrderByToUI(static_cast<short>(obj["orderBy"].toInt()));
    setLanguageToUI(obj["language"].toString());
    setFavoriteTagToUI(obj["favoriteTag"].toString());
    
    // 加载忽略规则列表
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

/**
 * @brief 获取当前UI中的所有设置
 * @return 包含所有设置的结构体
 */
set Settings::getSettings() {
    set target;
    target.ignoreLyric = ui.ignoreLyricBox->isChecked();  // 获取是否忽略歌词设置
    
    // 获取排序字段设置
    if (ui.titleSelect->isChecked()) {
        target.sortBy = toShort(SortByEnum::TITLE);
    } else if (ui.artistSelect->isChecked()) {
        target.sortBy = toShort(SortByEnum::ARTIST);
    } else if (ui.albumSelect->isChecked()) {
        target.sortBy = toShort(SortByEnum::ALBUM);
    }
    
    // 获取排序顺序设置
    if (ui.ascButton->isChecked()) {
        target.orderBy = toShort(OrderByEnum::ASC);
    } else if (ui.descButton->isChecked()) {
        target.orderBy = toShort(OrderByEnum::DESC);
    }
    
    target.language = ui.languageComboBox->currentText();    // 获取语言设置
    target.favoriteTag = ui.favoriteTagEdit->text();        // 获取收藏标签设置
    
    // 设置规则字符串并复制规则列表
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
        sortBy = toShort(SortByEnum::TITLE);
    }
    entity.sortBy = sortBy;
    switch (toSortBy(entity.sortBy)) {
    case SortByEnum::TITLE:
        ui.titleSelect->setChecked(true);
        break;
    case SortByEnum::ARTIST:
        ui.artistSelect->setChecked(true);
        break;
    case SortByEnum::ALBUM:
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
        Logger::Fatal("Error opening langinfo.json: " + file.errorString());
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
        orderBy = toShort(OrderByEnum::ASC);
    }
    entity.orderBy = orderBy;
    switch (toOrderBy(entity.orderBy)) {
    case OrderByEnum::ASC:
        ui.ascButton->setChecked(true);
        break;
    case OrderByEnum::DESC:
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
        Logger::Debug("Deleting rule at row: " + QString::number(item->row()));
        entity.rules.removeAt(item->row());
        ui.rulesWidget->removeRow(item->row());
    }
}

void Settings::addRule(const LyricIgnoreRule& singleton) {
    entity.rules.append(singleton);
    setIgnoreRulesToUI(entity.rules);
}
