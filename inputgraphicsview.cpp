#include "inputgraphicsview.h"
#include <QFileDialog>

InputGraphicsView::InputGraphicsView(QWidget *parent) : QGraphicsView(parent) {
	scene = new QGraphicsScene(this);
	setScene(scene);
}

void InputGraphicsView::setFilename(QString filename) {
	this->filename = filename;

	QPixmap pixmap(filename);
	pixmap = pixmap.scaledToHeight(int(0.95*height()), Qt::SmoothTransformation);

	scene->clear();
	scene->addPixmap(pixmap);
}

InputGraphicsView::~InputGraphicsView(){
	delete scene;
}

void InputGraphicsView::mousePressEvent(QMouseEvent *event){
	filename = QFileDialog::getOpenFileName(this, "Open Image", "../HeadPoseEstimation/data", "Images (*.png)");

	QPixmap pixmap(filename);
	pixmap = pixmap.scaledToWidth(int(0.95*width()), Qt::SmoothTransformation);

	scene->clear();
	scene->addPixmap(pixmap);
}

void InputGraphicsView::resizeEvent(QResizeEvent *event){
	QPixmap pixmap(filename);
	pixmap = pixmap.scaledToWidth(int(0.95*width()), Qt::SmoothTransformation);

	scene->clear();
	scene->addPixmap(pixmap);
}
