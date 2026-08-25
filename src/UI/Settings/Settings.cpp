#include "Settings.h"
#include <QJsonArray>
#include <QMessageBox>
#include "../../Services/Logger.h"

#include "../AddRuleWidget/AddRuleWidget.h"

Settings::Settings(const SettingsData& entity, QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	this->ui.rulesWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
	this->setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
	this->setWindowModality(Qt::ApplicationModal);

	// 从实体加载设置到UI控件
	setIgnoreLyricToUI(entity.ignoreLyric);
	setSortByToUI(entity.sortBy);
	setOrderByToUI(entity.orderBy);
	setLanguageToUI(entity.language);
	setFavoriteTagToUI(entity.favoriteTag);
	setRecursiveScanToUI(entity.recursiveScan);

	// 加载忽略规则列表
	QList<LyricIgnoreRule> rules = entity.rules;
	setIgnoreRulesToUI(rules);
}

SettingsData Settings::getSettings() {
	SettingsData target;
	target.ignoreLyric = ui.ignoreLyricBox->isChecked();

	if (ui.titleSelect->isChecked()) {
		target.sortBy = toShort(SortByEnum::TITLE);
	}
	else if (ui.artistSelect->isChecked()) {
		target.sortBy = toShort(SortByEnum::ARTIST);
	}
	else if (ui.albumSelect->isChecked()) {
		target.sortBy = toShort(SortByEnum::ALBUM);
	}

	if (ui.ascButton->isChecked()) {
		target.orderBy = toShort(OrderByEnum::ASC);
	}
	else if (ui.descButton->isChecked()) {
		target.orderBy = toShort(OrderByEnum::DESC);
	}

	target.language = ui.languageComboBox->currentText();
	target.favoriteTag = ui.favoriteTagEdit->text();

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
		}
		else {
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

void Settings::setRecursiveScanToUI(const bool recursiveScan) {
	entity.recursiveScan = recursiveScan;
	ui.recursiveScan->setChecked(recursiveScan);
}

void Settings::setIgnoreRulesToUI(QList<LyricIgnoreRule>& rules) {
	entity.rules = rules;
	ui.rulesWidget->setRowCount(static_cast<int>(rules.size()));
	int row = 0;
	for (LyricIgnoreRule& rule : rules) {
		rule.setRulesStr();
		ui.rulesWidget->setItem(
			row, 0, new QTableWidgetItem(LyricIgnoreRule::lyricRulesToString(rule.getRuleField())));
		ui.rulesWidget->setItem(
			row, 1, new QTableWidgetItem(LyricIgnoreRule::ignoreRulesToString(rule.getRuleType())));
		ui.rulesWidget->setItem(row, 2, new QTableWidgetItem(rule.getRuleName()));
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

void Settings::addRule(const LyricIgnoreRule& rule) {
	entity.rules.append(rule);
	setIgnoreRulesToUI(entity.rules);
}
