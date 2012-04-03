#ifndef INPUTGRAPHICSVIEW_H
#define INPUTGRAPHICSVIEW_H

#include <QGraphicsView>
#include <QGraphicsScene>

class InputGraphicsView : public QGraphicsView {
public:
	explicit InputGraphicsView(QWidget *parent = 0);
	~InputGraphicsView();
	virtual void mousePressEvent(QMouseEvent *event);
	virtual void resizeEvent(QResizeEvent *event);
	void setFilename(QString filename);

	QGraphicsScene *scene;
	QString filename;
};

#endif // INPUTGRAPHICSVIEW_H
