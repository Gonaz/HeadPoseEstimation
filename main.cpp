#include "yawdetector.h"
#include "pitchdetector.h"
#include <QDir>
#include <opencv2/core/core.hpp>
#include <iostream>

#include "image.h"
#include "landmarkreader.h"
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
double param = 0.005;

void crossValidateYaw(){
	unsigned long correct = 0;
	unsigned long wrong = 0;
	unsigned long error = 0;

	QString base = "../HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";
	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		foreach(QString image, images){
			std::cout << "Test " << image.toStdString() << std::endl;
			YawDetector yd(imageDir);
			yd.useLandmarks();
			int detectedYaw = yd(imageDir + "/" + image);
//			int detectedYaw = yd.newDetect(imageDir + "/" + image, param);
			int realYaw = yd.yaw(image);
			if(detectedYaw != realYaw){
				++wrong;
				error += abs(detectedYaw - realYaw);
			} else {
				++correct;
			}
			//			std::cout << "Estimation " << detectedYaw << "\t Real " << realYaw << std::endl;
			std::cout << std::endl;
		}
	}

	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
}

void crossValidatePitch(){
	QString base = "../HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";

	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		foreach(QString image, images){
			if(!image.contains("YR")){
				std::cout << "Test " << image.toStdString() << std::endl;
				PitchDetector pd(imageDir);
				pd(imageDir + "/" + image, param);
				std::cout << std::endl;
			}
		}
	}
}

/* TODO
	adaptive resize in image
	nose, mouth, eye detectie constraints opleggen
	=> positie van features zou overal bekend moeten zijn
	de yaw detector laten werken op de file -> sneller (cf. pitch detector)
	de pitch detector de pitch laten teruggeven
*/

int main(int argc, char *argv[]) {
//		YawDetector yd(true); //train the yawdetector
		crossValidateYaw(); // 171/245 met absolute error = 19.5306; orig 187/244 met absolute error = 8.68852

//	if(argc > 1){
//		param = QString(argv[1]).toDouble();
//	}

//	std::cout << param << std::endl;
//	std::cout << "Starting /home/jonas/Qt/HeadPoseEstimation-Debug/HeadPoseEstimation..." << std::endl;
//	crossValidateYaw();

	//		PitchDetector pd(true); //train the pitchdetector

	//	if(argc > 1){
	//		param = QString(argv[1]).toDouble(); //0.0346 is een goed waarde voor diff, voor div 0.14
	//	}

	//	std::cout << param << std::endl;
	//	std::cout << "Starting /home/jonas/Qt/HeadPoseEstimation-Debug/HeadPoseEstimation..." << std::endl;
	//	crossValidatePitch();
	return 0;
}
