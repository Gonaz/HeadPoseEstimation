#include "mouthtrainer.h"
#include "pitchdetector.h"
#include "image.h"
#include "landmarkreader.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDir>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

MouthTrainer::MouthTrainer() : positionFile("autoTest"){
}

void MouthTrainer::operator()(){
	auto positions = calculateRelativePositions();
	serialize(positions);
}

double MouthTrainer::features(QString filename){
	Mat image = imread(filename.toStdString());
	QString landMarkFile = filename;
	landMarkFile.replace(".png", ".lm2");
	LandMarkReader lmr(landMarkFile);
	auto point1 = lmr.fetchValue("Outer right eye corner");
	auto point2 = lmr.fetchValue("Inner right eyebrow");
	auto point3 = lmr.fetchValue("Nose tip");

	double n0 = point1.second/double(image.rows);
	double n1 = point2.second/double(image.rows);
	double n2 = point3.second/double(image.rows);

	if((n1 - n2) == 0){
		return 0;
	}
	return (n0 - n1) / (n1 - n2);
}

void MouthTrainer::serialize(QMap<QString, QPair<long, double> > positions){
	QFile file(positionFile);
	if(file.open(QIODevice::WriteOnly)){
		QTextStream stream(&file);

		for(int i=0; i<positions.count(); ++i){
			auto key = positions.keys().at(i);
			auto value = positions.value(key);

			stream << key << "\n" << value.first << "\n" << value.second << "\n";
		}
	}

	file.close();
}

QMap<QString, QPair<long, double> > MouthTrainer::calculateRelativePositions(){
	QMap<QString, QPair<long, double> > result;
	QStringList subdirs = QDir("../HeadPoseEstimation/data/").entryList();
	auto last = std::remove_if(subdirs.begin(), subdirs.end(), [](QString s){return !s.startsWith("bs");});

for(auto elem=subdirs.begin(); elem != last; ++elem){
	QDir imageDir("../HeadPoseEstimation/data/" + *(elem));
	QStringList images = imageDir.entryList(QStringList("*.png"));
	foreach(QString image, images){
		QString imagePath = "../HeadPoseEstimation/data/" + *(elem) + "/" + image;
		if(!image.contains("YR")){
			std::cout << "Detect " << imagePath.toStdString() << std::endl;
			long realPitch = PitchDetector::pitch(imagePath);
			auto values = features(imagePath);
			result[imagePath]= qMakePair(realPitch, values);
		}
	}
}

return result;
}
