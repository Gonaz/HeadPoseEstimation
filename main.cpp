#include "yawdetector.h"
#include "yawtrainer.h"
#include "mouthtrainer.h"
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
double param = 0.014;
//double param = 0.44;

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
			YawDetector yd = YawDetector(positionsFile).retainDir(imageDir);
			int detectedYaw = yd(imageDir + "/" + image);
			int realYaw = yd.yaw(image);
			if(detectedYaw != realYaw){
				++wrong;
				++wrongDir;
				error += abs(detectedYaw - realYaw);
				if(detectedYaw*realYaw < 0){
					++wrongDirection;
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

void crossValidatePitch(QString positionsFile){
	unsigned long correct = 0;
	unsigned long wrong = 0;
	unsigned long error = 0;
	QString base = "../HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";

	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		unsigned long correctDir = 0;
		unsigned long wrongDir = 0;
		foreach(QString image, images){
			if(!image.contains("YR")){
				PitchDetector pd = PitchDetector(positionsFile).retainDir(imageDir);
				int detectedPitch = pd(imageDir + "/" + image, param);
				int realPitch = pd.pitch(image);
				if(detectedPitch != realPitch){
					++wrong;
					++wrongDir;
					error += abs(detectedPitch - realPitch);
					//															std::cout << image.toStdString() << "\t" << realPitch << "\t" << detectedPitch << std::endl;
				} else {
					++correct;
					++correctDir;
				}
				if(detectedPitch * realPitch < -1){
					//					std::cout << "Wrong" << std::endl;
				}
			}
		}
		//		std::cout << imageDir.toStdString() << " -> " << correctDir << "/" << (correctDir + wrongDir) << std::endl;
	}
	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
}

//---------------------------------------------------------------------------------------------
//#include "test.h"
// TODO: remove this
//---------------------------------------------------------------------------------------------

/* TODO:
	adaptive resize in image
	nose, mouth, eye detectie constraints opleggen
	=> positie van features zou overal bekend moeten zijn
	de pitch detector de pitch laten teruggeven
*/

int main(int argc, char *argv[]) {
//	test5();
	//	YawTrainer yt = false;
	//	yt();
	//	crossValidateYaw("positionsYaw");
	//	crossValidateYaw("positionsYawOrig");
	//	crossValidateYaw("positionsYawBetter");

	//	PitchTrainer pt = false;
	//	pt();

	//	crossValidatePitch("mouthP");
	//	crossValidatePitch("positionsPitchNew");

						if(argc > 1){
							param = QString(argv[1]).toDouble(); //0.0346 is een goed waarde voor diff, voor div 0.14, 0.041 voor divcal
						}

						std::cout << param << std::endl;
						crossValidatePitch("gmTest2");

	//						MouthTrainer mt;
	//						mt();
	return 0;
}
