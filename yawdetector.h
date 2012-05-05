#ifndef YAWDETECTOR_H
#define YAWDETECTOR_H

#include <QString>
#include <QMap>
#include <QPair>
#include <QStringList>
#include <QVector>

class YawDetector {
public:
	YawDetector(bool train);
	YawDetector(QString retainDir);
	long operator()(QString image);
	long newDetect(QString filename, double fuzziness);
	static int yaw(QString filename);
	void useLandmarks();
private:
	QMap<QString, QPair<long, long> > positions;
	static const unsigned long ONE_MILLION = 1000000;
	QString positionFile;
	bool landmarks;

	QMap<QString, QPair<long, long> > calculateRelativePositions();
	QPair<long, QString> getBest(QMultiMap<long, QString> scores, int number=0);
	size_t size(QPair<long, long> pair);
	void serialize(QMap<QString, QPair<long, long> > positions);
	QMap<QString, QPair<long, long> > deserialize();
	QPair<long, long> positionsFromFile(QString filename);

	QVector<long> detectYaw(QString filename, double fuzziness);
	bool containsTies(QVector<long> vec);
};

#endif // YAWDETECTOR_H
