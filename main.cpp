#include "yawdetector.h"
#include <QDir>
#include <opencv2/core/core.hpp>
#include <iostream>

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
			std::cout << "Estimation " << detectedYaw << "\t Real " << realYaw << std::endl;
			if(detectedYaw != realYaw){
				++wrong;
				error += abs(detectedYaw - realYaw);
			} else {
				++correct;
			}
		}
	}

	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
}

int main(int argc, char *argv[]) {
//	YawDetector yd(true); //train the yawdetector
	crossValidateYaw();
	return 0;
}
