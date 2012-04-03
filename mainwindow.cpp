#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QImage>
#include <QGraphicsScene>
#include <QDir>
#include <QPair>
#include <iostream>


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
	ui->setupUi(this);
	QString filename = "/home/jonas/Qt/HeadPoseEstimation/data/bs000/bs000_PR_SD_0.png";
	ui->input->setFilename(filename);

	QImage image1(filename);

	QMultiMap<long, QString> scores = compare(image1);
	std::cout << getBest(scores).first << "->" << getBest(scores).second.toStdString() << std::endl;
	std::cout << getBest(scores, 1).first << "->" << getBest(scores, 1).second.toStdString() << std::endl;

	QStringList filenames;
	filenames << getBest(scores,1).second << getBest(scores,2).second << getBest(scores,3).second << getBest(scores,4).second;
	setOutput(filenames);
}

MainWindow::~MainWindow() {
	delete ui;
}

void MainWindow::setOutput(QStringList filenames){
	QGraphicsScene *scene1 = new QGraphicsScene();
	QPixmap pixmap(filenames.at(0));
	pixmap = pixmap.scaledToHeight(450, Qt::SmoothTransformation);
	scene1->addPixmap(pixmap);
	ui->output1->setScene(scene1);
	ui->outputLabel1->setText(filenames.at(0));

	QGraphicsScene *scene2 = new QGraphicsScene();
	pixmap = QPixmap(filenames.at(1));
	pixmap = pixmap.scaledToHeight(450, Qt::SmoothTransformation);
	scene2->addPixmap(pixmap);
	ui->output2->setScene(scene2);
	ui->outputLabel2->setText(filenames.at(1));

	QGraphicsScene *scene3 = new QGraphicsScene();
	pixmap = QPixmap(filenames.at(2));
	pixmap = pixmap.scaledToHeight(450, Qt::SmoothTransformation);
	scene3->addPixmap(pixmap);
	ui->output3->setScene(scene3);
	ui->outputLabel3->setText(filenames.at(2));

	QGraphicsScene *scene4 = new QGraphicsScene();
	pixmap = QPixmap(filenames.at(3));
	pixmap = pixmap.scaledToHeight(450, Qt::SmoothTransformation);
	scene4->addPixmap(pixmap);
	ui->output4->setScene(scene4);
	ui->outputLabel4->setText(filenames.at(3));
}



QMultiMap<long, QString> MainWindow::compare(QImage image1){
	QString base = "/home/jonas/Qt/HeadPoseEstimation/data";

	QDir dir(base);
	QStringList subdirs = dir.entryList();
	for(int i=0; i<subdirs.count(); ++i){
		if(!subdirs.at(i).startsWith("bs000")){
			subdirs.removeAt(i--);
		}
	}

	QMultiMap<long, QString> scores;
	foreach(QString subdir, subdirs){
		std::cout << "subdir " << subdir.toStdString() << std::endl;
		QDir imageDir(base + "/" + subdir);
		QStringList images = imageDir.entryList(QStringList("*.png"));
		foreach(QString image, images){
			QString imagePath = base + "/" + subdir + "/" + image;
			QImage image2(imagePath);
			//long score = compare(image1, image2);
			long score = 0;
			scores.insertMulti(abs(score), imagePath);
		}
	}

	return scores;
}

QPair<long, QString> MainWindow::getBest(QMultiMap<long, QString> scores, int number){
	QList<long> keys = scores.keys();
	qSort(keys.begin(), keys.end());
	return qMakePair(keys.at(number), scores.value(keys.at(number)));
}
