#include "Settings.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QMessageBox>
#include <utility>
#include "../../Services/Logger.h"

#include "../AddRuleWidget/AddRuleWidget.h"

// LyricIgnoreRule 不再继承 QObject，原先经它传递到本翻译单元的 PROPERTIES 名字改为显式引入
using PROPERTIES::AccentColor;
using PROPERTIES::OrderByEnum;
using PROPERTIES::SortByEnum;
using PROPERTIES::ThemeMode;
using PROPERTIES::toOrderBy;
using PROPERTIES::toShort;
using PROPERTIES::toSortBy;

Settings::Settings(const SettingsData& entity, QWidget* parent) : QWidget(parent) {
	ui.setupUi(this);
	this->ui.rulesWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
	// 只给 hint 位时 Qt 会按 Qt::Widget 处理并 setParent(parentWidget())，
	// 使设置页变成主窗口的子控件；显式声明 Qt::Window 才是独立顶层窗口（parent 仍保留，便于居中与销毁）
	this->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowContextHelpButtonHint);
	this->setWindowModality(Qt::ApplicationModal);

	// 主题下拉：itemData 直接存 PROPERTIES 编码，避免依赖下拉项顺序
	ui.themeLD->addItem(tr("明亮"), static_cast<int>(PROPERTIES::toShort(ThemeMode::LIGHT)));
	ui.themeLD->addItem(tr("暗色"), static_cast<int>(PROPERTIES::toShort(ThemeMode::DARK)));
	ui.themeColor->addItem(tr("蓝色"), static_cast<int>(PROPERTIES::toShort(AccentColor::BLUE)));
	ui.themeColor->addItem(tr("绿色"), static_cast<int>(PROPERTIES::toShort(AccentColor::GREEN)));
	ui.themeColor->addItem(tr("紫色"), static_cast<int>(PROPERTIES::toShort(AccentColor::PURPLE)));
	ui.themeColor->addItem(tr("深色"), static_cast<int>(PROPERTIES::toShort(AccentColor::DARK)));

	// 从实体加载设置到UI控件
	setIgnoreLyricToUI(entity.ignoreLyric);
	setSortByToUI(entity.sortBy);
	setOrderByToUI(entity.orderBy);
	setLanguageToUI(entity.language);
	setFavoriteTagToUI(entity.favoriteTag);
	setRecursiveScanToUI(entity.recursiveScan);
	setLogToFileToUI(entity.logToFile);
	setThemeToUI(entity.themeMode, entity.accentColor);

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
	// 递归扫描此前只写不读，导致配置永远为初始值
	target.recursiveScan = ui.recursiveScan->isChecked();
	target.logToFile = ui.logToFileBox->isChecked();
	target.themeMode = static_cast<short>(ui.themeLD->currentData().toInt());
	target.accentColor = static_cast<short>(ui.themeColor->currentData().toInt());

	// 设置规则字符串并复制规则列表
	for (auto& rule : entity.rules) {
		rule.setRulesStr();
	}
	target.rules = this->entity.rules;
	return target;
}

void Settings::setIgnoreLyricToUI(const bool ignoreLyric) {
	entity.ignoreLyric = ignoreLyric;
	ui.ignoreLyricBox->setChecked(ignoreLyric);
}

void Settings::setSortByToUI(short sortBy) {
	// 排序字段编码为 0x01=标题 / 0x02=艺术家 / 0x03=专辑（见 PROPERTIES::toShort），越界回退标题
	if (sortBy < toShort(SortByEnum::TITLE) || sortBy > toShort(SortByEnum::ALBUM)) {
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
		// 语言配置缺失只影响语言下拉框，提示后保持原有语言
		QMessageBox::critical(this, tr("错误"), tr("无法打开语言文件"));
		Logger::Error("Error opening langinfo.json: " + file.errorString());
		return;
	}
	const QJsonDocument langinfo = QJsonDocument::fromJson(file.readAll());
	QJsonArray langArray = langinfo.array();
	for (const QJsonValue& lang : std::as_const(langArray)) {
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
	// 升降序编码为 0x11=升序 / 0x12=降序（见 PROPERTIES::toShort），越界回退升序
	if (orderBy < toShort(OrderByEnum::ASC) || orderBy > toShort(OrderByEnum::DESC)) {
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

void Settings::setLogToFileToUI(const bool logToFile) {
	entity.logToFile = logToFile;
	ui.logToFileBox->setChecked(logToFile);
}

void Settings::setThemeToUI(const short themeMode, const short accentColor) {
	entity.themeMode = themeMode;
	entity.accentColor = accentColor;

	const int modeIndex = ui.themeLD->findData(static_cast<int>(entity.themeMode));
	if (modeIndex >= 0) {
		ui.themeLD->setCurrentIndex(modeIndex);
	}
	const int accentIndex = ui.themeColor->findData(static_cast<int>(entity.accentColor));
	if (accentIndex >= 0) {
		ui.themeColor->setCurrentIndex(accentIndex);
	}
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

void Settings::on_addRule_clicked() {
	// 挂到设置窗口下并设置关闭即销毁，避免每次添加规则都泄漏一个窗口
	const auto widget = new AddRuleWidget(this);
	widget->setAttribute(Qt::WA_DeleteOnClose);
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
