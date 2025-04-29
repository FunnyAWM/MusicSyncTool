#ifndef OPERATIONRESULT_H
#define OPERATIONRESULT_H


#include "ui_OperationResult.h"

class OperationResult final : public QDialog {
	Q_OBJECT

public:
    explicit OperationResult(QWidget* parent = nullptr);
	void setText(const QString& text) const;
	~OperationResult() override = default;

private:
	Ui::OperationResultClass ui;
};

#endif // OPERATIONRESULT_H
