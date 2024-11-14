//
// Created by 11429 on 24-11-13.
//

// You may need to build the project (run Qt uic code generator) to get "ui_MainWindow.h" resolved

#include "mainwindow.h"

#include <iostream>

#include <QFileDialog>
#include "ui_MainWindow.h"
#include <taglib/flacfile.h>
#include <taglib/tag.h>



MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // db = QSqlDatabase::addDatabase("QSQLITE", "musicInfo");
    // db.setDatabaseName("musicinfo.db");
    // if (db.open())
    // {
    //     QString sql = "CREATE TABLE IF NOT EXISTS musicInfo (id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT)";
    //     query = QSqlQuery(db);
    //     query.prepare(sql);
    //     if (query.exec())
    //     {
    //         qDebug() << "Successfully created database";
    //     }
    //     sql = "INSERT INTO musicInfo(name) VALUES('Peter')";
    //     query = QSqlQuery(db);
    //     query.prepare(sql);
    //     if (query.exec())
    //     {
    //         sql = "SELECT * FROM musicInfo";
    //         query = QSqlQuery(db);
    //         query.prepare(sql);
    //     }
    //
    // }
    // else
    // {
    //     qDebug() << "Failed to open database";
    // }
    TagLib::FLAC::File ref("Alan Walker;Au;Ra;Tomine Harket - Darkside.flac");
    TagLib::String str = ref.tag()->title();
    std::cout << str.toCString();
}


MainWindow::~MainWindow() { delete ui; }

void MainWindow::on_actionOpenFolder_triggered(bool checked)
{
    QFileDialog fileDialog;
    fileDialog.setFileMode(QFileDialog::FileMode::Directory);
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setViewMode(QFileDialog::Detail);
    QDir dir = fileDialog.getExistingDirectory();
    std::cout << dir.absolutePath().toStdString() << std::endl;
}
