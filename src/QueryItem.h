#pragma once
#include <QString>

class QueryItem {
private:
	QString column;
	QString value;

public:
	QueryItem(QString column, QString value) : column(std::move(column)), value(std::move(value)) {}
	[[nodiscard]] QString getColumn() const { return column; }
	[[nodiscard]] QString getValue() const { return value; }
};
