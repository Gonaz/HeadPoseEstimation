#include "yawdetector.h"
#include "pitchdetector.h"
#include <QDir>
#include <opencv2/core/core.hpp>
#include <iostream>

#include "image.h"
#include <opencv2/highgui/highgui.hpp>

using namespace cv;

void crossValidateYaw(){
	unsigned long correct = 0;
	unsigned long wrong = 0;
	unsigned long error = 0;

	QString base = "/home/jonas/Qt/HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";
	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		foreach(QString image, images){
			std::cout << "Detect " << image.toStdString() << std::endl;
			YawDetector yd(imageDir);
			int detectedYaw = yd(imageDir + "/" + image);
			int realYaw = yd.yaw(image);
			if(detectedYaw != realYaw){
				++wrong;
				error += abs(detectedYaw - realYaw);
			} else {
				++correct;
			}
			std::cout << "Estimation " << detectedYaw << "\t Real " << realYaw << std::endl;
		}
	}

	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
}

void crossValidatePitch(){
	PitchDetector pd(false);
	QStringList subdirs = QDir("../HeadPoseEstimation/data/").entryList();
	auto last = std::remove_if(subdirs.begin(), subdirs.end(), [](QString s){return !s.startsWith("bs");});

for(auto elem=subdirs.begin(); elem != last; ++elem){
	QDir imageDir("../HeadPoseEstimation/data/" + *(elem));
	QStringList images = imageDir.entryList(QStringList("*.png"));
	foreach(QString image, images){
		QString imagePath = "../HeadPoseEstimation/data/" + *(elem) + "/" + image;
		if(!image.contains("YR")){
			std::cout << "Test " << image.toStdString() << std::endl;
			pd(imagePath, double(0.001));
			std::cout << std::endl;
		}
	}
}
}

int main(int argc, char *argv[]) {
	//	YawDetector yd(true); //train the yawdetector
	//	crossValidateYaw();

//		PitchDetector pd(true); //train the pitchdetector
	crossValidatePitch();
	return 0;
}
