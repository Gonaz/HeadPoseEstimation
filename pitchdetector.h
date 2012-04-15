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
	long operator()(QString filename, float fuziness=0.001);
	static long pitch(QString filename);
private:
	QMap<long, QVector<float> > positions;

	cv::vector<float> detectFeatures(cv::Mat image);
	double distanceMouthNose(cv::vector<float> features, cv::Mat image);
	double distanceNoseEye(cv::vector<float> features, cv::Mat image);
	QMap<long, QVector<float> > calculateRelativePositions();
	QVector<long> test(QString filename, float fuzziness);
	void serialize(QMap<long, QVector<float> > result);
	QMap<long, QVector<float> > deserialize();
};

#endif // PITCHDETECTOR_H
