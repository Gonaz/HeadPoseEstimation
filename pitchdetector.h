#ifndef PITCHDETECTOR_H
#define PITCHDETECTOR_H

#include <QString>
#include <QMap>
#include <QPair>
#include <QVector>
#include <opencv2/core/core.hpp>

class PitchDetector {
public:
	PitchDetector(QString positionFile);
	PitchDetector& retainDir(QString retainDir);
	long operator()(QString filename, double fuziness=0.001);
	static long pitch(QString filename);
private:
	QString positionFile;
	QMap<QString, QPair<long, double> > positions;

	QVector<long> detectPitch(QString filename, double fuzziness);
	QMap<QString, QPair<long, double> > deserialize();
	double position(QString filename);
	bool containsTies(QVector<long> vec);
};

#endif // PITCHDETECTOR_H
