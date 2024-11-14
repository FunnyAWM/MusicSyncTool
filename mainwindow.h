//
// Created by 11429 on 24-11-13.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


QT_BEGIN_NAMESPACE

namespace Ui
{
    class MainWindow;
}

QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    Ui::MainWindow *ui;
    // QSqlDatabase db;
    // QSqlQuery query;
    // QSqlError error;

private slots:
    void on_actionOpenFolder_triggered(bool);
};


#endif //MAINWINDOW_H
