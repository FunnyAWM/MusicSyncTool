#ifndef SHOWDUPE_H
#define SHOWDUPE_H

#include <QDialog>
#include "ui_ShowDupe.h"

class ShowDupe : public QDialog {
	Q_OBJECT

public:
	explicit ShowDupe(QWidget* parent = nullptr);
	void add(const QString&) const;
	~ShowDupe() override = default;

private:
	Ui::ShowDupeClass ui;
};

#endif // SHOWDUPE_H
