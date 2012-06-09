#include "yawdetector.h"
#include "yawtrainer.h"
#include "pitchdetector.h"
#include "pitchtrainer.h"
#include <QDir>
#include <opencv2/core/core.hpp>
#include <iostream>
#include "image.h"
#include "landmarkreader.h"
#include <opencv2/highgui/highgui.hpp>
#include <QTime>

using namespace cv;
double param = 0.15; //own
//double param = 0.41; //landmarks
//double param = 6.5; //euler (3.9)

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
		foreach(QString image, images){
			YawDetector yd = YawDetector(positionsFile).retainDir(imageDir);
			int detectedYaw = yd(imageDir + "/" + image);
			int realYaw = yd.yaw(image);
			if(detectedYaw != realYaw){
				++wrong;
				error += abs(detectedYaw - realYaw);
				if(detectedYaw*realYaw < 0){
					++wrongDirection;
				}
			} else {
				++correct;
			}
		}
	}

	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
	std::cout << "Wrong directions " << wrongDirection << std::endl;
}

void crossValidatePitch(QString positionsFile){
	unsigned long correct = 0;
	unsigned long wrong = 0;
	unsigned long error = 0;
	unsigned long wrongDirection = 0;

	QString base = "../HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";

	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		foreach(QString image, images){
			if(!image.contains("YR")){
				PitchDetector pd = PitchDetector(positionsFile).retainDir(imageDir);
				int detectedPitch = pd(imageDir + "/" + image, param);
				int realPitch = pd.pitch(image);
				if(detectedPitch != realPitch){
					++wrong;
					error += abs(detectedPitch - realPitch);
					if(detectedPitch * realPitch < 0){
						++wrongDirection;
					}
				} else {
					++correct;
				}
			}
		}
	}
	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
	std::cout << "Wrong directions " << wrongDirection << std::endl;
}

int main(int argc, char **argv) {
//		YawTrainer yt = false;
//		yt();
//		crossValidateYaw("positionsYaw");
//		crossValidateYaw("positionsYawOrig");

		PitchTrainer pt = false;
		pt();

		crossValidatePitch("positionsPitchNew");

//	if(argc > 1){
//		param = QString(argv[1]).toDouble();
//	}

//	std::cout << param << std::endl;
//	crossValidatePitch("output1");
	return 0;
}
