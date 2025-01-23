#include "LoadingPage.h"
#include "MusicSyncTool.h"

/*
 * @brief Constructor for LoadingPage
 */
LoadingPage::LoadingPage(QWidget *parent) : QWidget(parent), total(0) {
    ui.setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
}
/*
 * @brief Set the title of the LoadingPage
 * @param title The title to set
 */
void LoadingPage::setTitle(const QString& title) { this->setWindowTitle(title); }

void LoadingPage::showPage() {
    ui.progressBar->setValue(0);
    show();
}
/*
 * @brief Stop the LoadingPage
 */
void LoadingPage::stopPage() { close(); }
/*
 * @brief Set the progress of the LoadingPage
 * @param value The value to set
 */
void LoadingPage::setProgress(const qsizetype value) const { ui.progressBar->setValue((static_cast<double>(value) / total) * 100); }
/*
 * @brief Set the total of the LoadingPage
 * @param total The total to set
 */
void LoadingPage::setTotal(const qsizetype total) { this->total = total; }
