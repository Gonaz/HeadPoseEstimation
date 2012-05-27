#ifndef MOUTHTRAINER_H
#define MOUTHTRAINER_H

#include <QString>
#include <QMap>
#include <QPair>
#include <opencv2/core/core.hpp>

class MouthTrainer {
public:
	MouthTrainer();
	void operator()();
private:
	QString positionFile;

	double features(QString filename);
	QMap<QString, QPair<long, double> > calculateRelativePositions();
	void serialize(QMap<QString, QPair<long, double> > positions);
};

#endif // MOUTHTRAINER_H
