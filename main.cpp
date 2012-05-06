#include "yawdetector.h"
#include "yawtrainer.h"
#include "pitchdetector.h"
#include <QDir>
#include <opencv2/core/core.hpp>
#include <iostream>

#include "image.h"
#include "landmarkreader.h"
#include <opencv2/highgui/highgui.hpp>
#include <QTime>

using namespace cv;
double param = 0.005;

void crossValidateYaw(QString positionsFile){
	unsigned long correct = 0;
	unsigned long wrong = 0;
	unsigned long error = 0;
	unsigned long wrongDirection = 0;

	QString base = "../HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";
	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		unsigned long correctDir = 0;
		unsigned long wrongDir = 0;
		foreach(QString image, images){
			//			std::cout << "Test " << image.toStdString() << std::endl;
			YawDetector yd = YawDetector(positionsFile).retainDir(imageDir);
			int detectedYaw = yd(imageDir + "/" + image);
			int realYaw = yd.yaw(image);
			if(detectedYaw != realYaw){
				++wrong;
				++wrongDir;
				error += abs(detectedYaw - realYaw);
				if(detectedYaw*realYaw < 0){
					++wrongDirection;
					//					 std::cout << image.toStdString() << std::endl;
					//					 std::cout << realYaw << " -> " << detectedYaw << std::endl;
				}
			} else {
				++correct;
				++correctDir;
			}
		}
		//		std::cout << imageDir.toStdString() << " -> " << correctDir << "/" << (correctDir + wrongDir) << std::endl;
	}

	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
	std::cout << "Wrong directions " << wrongDirection << std::endl;
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

void test(){
	QString filename = "../HeadPoseEstimation/data/bs000/bs000_YR_R90_0.png";
	Mat image = imread(filename.toStdString());
	vector<Rect> nose = Image::detectNose(image);
	std::cout << nose.size() << std::endl;
	if(nose.size() > 0){
		rectangle(image, nose.at(0), Scalar(0, 200, 0), 3);

		resize(image, image, Size(image.cols*0.6, image.rows*0.6));
		imshow("Nose", image);
		waitKey();
	}
}

/* TODO
	adaptive resize in image
	nose, mouth, eye detectie constraints opleggen
	=> positie van features zou overal bekend moeten zijn
	de pitch detector de pitch laten teruggeven
*/

int main(int argc, char *argv[]) {
//	test();
		YawTrainer yt = false;
		yt();
		crossValidateYaw("positionsYaw");
	//	crossValidateYaw("positionsYawOrig");
	//	crossValidateYaw("positionsYawBetter");

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
