#include "MusicSyncTool.h"
#include <QFile>
#include <taglib/tag.h>
#include <taglib/flacfile.h>
#include <iostream>

MusicSyncTool::MusicSyncTool(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    TagLib::FLAC::File ref("D:/“Ù¿÷/Avicii;Aloe Blacc - Wake Me Up.flac");
}

MusicSyncTool::~MusicSyncTool()
{}
