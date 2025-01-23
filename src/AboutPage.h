#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QDialog>
#include "ui_AboutPage.h"

class AboutPage final : public QDialog {
	Q_OBJECT

public:
	explicit AboutPage(QWidget* parent = nullptr);
	~AboutPage() override = default;

private:
	Ui::AboutPageClass ui;
};

#endif // ABOUTPAGE_H
