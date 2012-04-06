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
				std::cout << "\t verkeerd" << std::endl;
				++wrong;
				error += abs(detectedYaw - realYaw);
			} else {
				std::cout << "\t juist" << std::endl;
				++correct;
			}
			std::cout << "Estimation " << detectedYaw << "\t Real " << realYaw << std::endl;
		}
	}

	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
}

void crossValidatePitch(){
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
			PitchDetector pd(imageDir);
			int detectedPitch = pd(imageDir + "/" + image);
			int realPitch = pd.pitch(image);
			if(detectedPitch != realPitch){
				std::cout << "\t verkeerd" << std::endl;
				++wrong;
				error += abs(detectedPitch - realPitch);
			} else {
				std::cout << "\t juist" << std::endl;
				++correct;
			}
			std::cout << "Estimation " << detectedPitch << "\t Real " << realPitch << std::endl;
		}
	}

	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;

}

int main(int argc, char *argv[]) {
//	YawDetector yd(true); //train the yawdetector
//	crossValidateYaw();

	QString base = "../HeadPoseEstimation/data/bs000/";
	QStringList images = QDir(base).entryList(QStringList("*.png"));
	foreach(QString image, images){
		PitchDetector pd(false);
		std::cout << image.toStdString() << "#" << pd.pitch(base+image) << std::endl;
	}

	return 0;
}
