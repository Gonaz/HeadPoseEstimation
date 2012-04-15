#ifndef YAWDETECTOR_H
#define YAWDETECTOR_H

#include <QString>
#include <QMap>
#include <QPair>
#include <QStringList>

class YawDetector {
public:
	YawDetector(bool train);
	YawDetector(QString retainDir);
	long operator()(QString image);
	static int yaw(QString filename);
private:
	QMap<QString, QPair<long, long> > positions;
	static const unsigned long ONE_MILLION = 1000000;

	QMap<QString, QPair<long, long> > calculateRelativePositions();
	QPair<long, QString> getBest(QMultiMap<long, QString> scores, int number=0);
	size_t size(QPair<long, long> pair);
	void serialize(QMap<QString, QPair<long, long> > positions);
	QMap<QString, QPair<long, long> > deserialize();
};

#endif // YAWDETECTOR_H