#ifndef PITCHTRAINER_H
#define PITCHTRAINER_H

#include <QMap>
#include <QPair>
#include <opencv2/core/core.hpp>

class PitchTrainer {
public:
	PitchTrainer(bool landmarks);
	void operator()();
private:
	QString positionFile;
	std::function<cv::vector<double> (QString)> features;

	cv::vector<double> readFeatures(QString filename);
	cv::vector<double> detectFeatures(QString filename);
	QMap<QString, QPair<long, double> > calculateRelativePositions();
	void serialize(QMap<QString, QPair<long, double> > positions);
	double distanceMouthNose(cv::vector<double> features, cv::Mat image);
	double distanceNoseEye(cv::vector<double> features, cv::Mat image);
};

#endif // PITCHTRAINER_H
