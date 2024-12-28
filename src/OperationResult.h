#ifndef OPERATIONRESULT_H
#define OPERATIONRESULT_H

#include <QDialog>
#include "ui_OperationResult.h"

class OperationResult : public QDialog {
	Q_OBJECT

public:
	OperationResult(QWidget* parent = nullptr);
	void setText(const QString& text) const;
	~OperationResult() override = default;

private:
	Ui::OperationResultClass ui;
};

#endif // OPERATIONRESULT_H
