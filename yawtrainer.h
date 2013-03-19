#ifndef YAWTRAINER_H
#define YAWTRAINER_H

#include <QMap>
#include <QPair>
#include <functional>

class YawTrainer {
public:
	YawTrainer(bool landmarks);
	void operator()();
	friend class YawDetector;
private:
	QString positionFile;
	std::function<QPair<long, long> (QString)> positions;

	QMap<QString, QPair<long, long> > calculateRelativePositions();
	QPair<long, long> detectPositions(QString imagePath);
	QPair<long, long> readPositions(QString imagePath);
	void serialize(QMap<QString, QPair<long, long> > positions);
};

#endif // YAWTRAINER_H
