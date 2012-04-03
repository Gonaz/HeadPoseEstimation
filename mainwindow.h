#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMultiMap>

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
	QImage grayscale(QImage image);
	long compare(QImage image1, QImage image2);
	QMultiMap<long, QString> compare(QImage image);
	QPair<long, QString> getBest(QMultiMap<long, QString> scores, int number=0);
	void setOutput(QStringList filenames);
private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
