#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MusicSyncTool.h"


class MusicSyncTool : public QMainWindow
{
    Q_OBJECT

public:
    MusicSyncTool(QWidget *parent = nullptr);
    ~MusicSyncTool();

private:
    Ui::MusicSyncToolClass ui;
};
