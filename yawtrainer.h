#ifndef YAWTRAINER_H
#define YAWTRAINER_H

#include <QMap>
#include <QPair>

class YawTrainer {
public:
	YawTrainer(bool landmarks);
	void operator()();
private:
	bool landmarks;
	QString positionFile;
	std::function<QPair<long, long> (QString)> positions;

	QMap<QString, QPair<long, long> > calculateRelativePositions();
	QPair<long, long> detectPositions(QString imagePath);
	QPair<long, long> detectBetterPositions(QString imagePath);
	QPair<long, long> readPositions(QString imagePath);
	void serialize(QMap<QString, QPair<long, long> > positions);
};

#endif // YAWTRAINER_H
