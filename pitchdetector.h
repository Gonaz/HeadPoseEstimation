#ifndef PITCHDETECTOR_H
#define PITCHDETECTOR_H

#include <QString>
#include <QMap>
#include <QPair>
#include <QVector>
#include <opencv2/core/core.hpp>

class PitchDetector {
public:
	PitchDetector(bool train);
	PitchDetector(QString retainDir);
	long operator()(QString filename, double fuziness=0.001);
	static long pitch(QString filename);
private:
	QMap<QString, QPair<long, double> > positions;

	cv::vector<double> detectFeatures(cv::Mat image);
	double distanceMouthNose(cv::vector<double> features, cv::Mat image);
	double distanceNoseEye(cv::vector<double> features, cv::Mat image);
	QMap<QString, QPair<long, double> > calculateRelativePositions();
	QVector<long> detectPitch(QString filename, double fuzziness);
	void serialize(QMap<QString, QPair<long, double> > result);
	QMap<QString, QPair<long, double> > deserialize();
};

#endif // PITCHDETECTOR_H
