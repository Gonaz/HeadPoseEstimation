#include "yawdetector.h"
#include "yawtrainer.h"
#include "pitchdetector.h"
#include "pitchtrainer.h"
#include "facialnormal.h"
#include <QDir>
#include <opencv2/core/core.hpp>
#include <iostream>
#include "image.h"
#include "landmarkreader.h"
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
double param = 0.15; //own
//double param = 0.41; //landmarks
//double param = 6.5; //euler

void crossValidateYaw(QString positionsFile){
	unsigned long correct = 0;
	unsigned long wrong = 0;
	unsigned long error = 0;
	unsigned long wrongDirection = 0;
	unsigned long wrongFrontal = 0;

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
				if(detectedYaw == 0 and realYaw != 0){
					++wrongFrontal;
				} else if(detectedYaw != 0 and realYaw == 0){
					++wrongFrontal;
				}
			} else {
				++correct;
			}
		}
	}

	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
	std::cout << "Wrong directions " << wrongDirection << std::endl;
	std::cout << "Wrong frontal " << wrongFrontal << std::endl;
}

void crossValidatePitch(QString positionsFile){
	unsigned long correct = 0;
	unsigned long wrong = 0;
	unsigned long error = 0;
	unsigned long wrongDirection = 0;
	unsigned long wrongNeutral = 0;

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
					if(detectedPitch == 0 and realPitch != 0){
						++wrongNeutral;
					} else if(detectedPitch != 0 and realPitch == 0){
						++wrongNeutral;
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
	std::cout << "Wrong neutral " << wrongNeutral << std::endl;
}

int main(int argc, char **argv) {
	if(argc > 1){
		//Get the filename from the arguments
		QString filename = QString::fromAscii(argv[1]);

		//Print the filename
		std::cout << filename.toStdString() << std::endl;

		//Read the image
		cv::Mat image = imread(filename.toStdString());

		//If the image is a frontal view then detect the pitch
		if(Image::isFrontalView(image)){
			PitchDetector pd("positionsPitch");
			std::cout << "Pitch " << pd(filename) << std::endl;
		}

		//Detect the yaw
		YawDetector yd("positionsYaw");
		std::cout << "Yaw " << yd(filename) << std::endl;
	} else {
		//Make a yaw trainer that does use the landmarks
		YawTrainer yt = false;
		//Train the yaw detector
		yt();

		//Cross validate with the own detection
		crossValidateYaw("positionsYaw");
		//Cross validate with landmarks
		crossValidateYaw("positionsYawOrig");

		//Make a pitch trainer that doesn't use the landmarks
		PitchTrainer pt = true;
		//Train the pitch detector
		pt();

		//The fuzziness parameter, this is a good value for the own detection
		param = 0.15;
		//Cross validate with the own detection
		crossValidatePitch("positionsPitch");

		//The fuzziness parameter, this is a good value for the ptich detector that uses the landmarks
		param = 0.41;
		//Cross validate with landmarks
		crossValidatePitch("positionsPitchOrig");
	}

	//Use the facial normal  (http://mmlab.disi.unitn.it/wiki/index.php/Head_Pose_Estimation_using_OpenCV)
	//facialNormal();
	return 0;
}
