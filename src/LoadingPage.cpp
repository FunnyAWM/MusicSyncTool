#include "LoadingPage.h"
#include "MusicSyncTool.h"
#include <cstdlib>
#include <ctime>

/*
 * @brief Constructor for LoadingPage
 */
LoadingPage::LoadingPage(QWidget* parent) : QWidget(parent), total(0) {
    ui.setupUi(this);
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowModality(Qt::ApplicationModal);
    this->setWindowIcon(QIcon(":/MusicSyncTool.ico"));
    setRandomTitle();
}
void LoadingPage::setTitle(const QString& title) const { ui.label->setText(title); }
void LoadingPage::setRandomTitle() const {
    srand(static_cast<unsigned>(time(nullptr)));
    QFile file(QCoreApplication::applicationDirPath() + "/resources/titles.txt");
    if (file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        const int totalTitle = stream.readLine().toInt();
        const int titleIndex = rand() % totalTitle;
        for (int i = 0; i < titleIndex; i++) {
            stream.readLine();
        }
        const QString title = stream.readLine();
        setTitle(title);
        file.close();
    }
}

void LoadingPage::showPage() {
	ui.progressBar->setValue(0);
    setRandomTitle();
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
void LoadingPage::setProgress(const qsizetype value) const {
	ui.progressBar->setValue(static_cast<double>(value) / total * 100);
}

/*
 * @brief Set the total of the LoadingPage
 * @param total The total to set
 */
void LoadingPage::setTotal(const qsizetype total) { this->total = total; }
