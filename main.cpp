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
double param = 0.014;

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
										std::cout << image.toStdString() << "\t" << realPitch << "\t" << detectedPitch << std::endl;
				} else {
					++correct;
					++correctDir;
				}
			}
		}
						std::cout << imageDir.toStdString() << " -> " << correctDir << "/" << (correctDir + wrongDir) << std::endl;
	}
	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
}

Point relative(Point original, Mat image){
	return Point(original.x/double(image.cols)*100, original.y/double(image.rows)*100);
}

void test2(){
	QString base = "../HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";
	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		foreach(QString image, images){
			if(!image.contains("YR")){
				QString filename = imageDir + "/" + image;
				std::cout << filename.toStdString() << std::endl;
				Mat image = imread(filename.toStdString());
				vector<Rect> eyes = Image::detectEyes(image);
				vector<Rect> nose = Image::detectNose(image);
				vector<Rect> mouth = Image::detectMouth(image);

				std::cout << "Eyes " << eyes.size() << std::endl;
				for(int i=0; i<eyes.size(); ++i){
					rectangle(image, eyes.at(i), Scalar(200, 0, 0), 3);
				}

				std::cout << "Nose " << nose.size() << std::endl;
				for(int i=0; i<nose.size(); ++i){
					rectangle(image, nose.at(i), Scalar(0, 200, 0), 3);
				}

				std::cout << "Mouth " << mouth.size() << std::endl;
				for(int i=0; i<mouth.size(); ++i){
					rectangle(image, mouth.at(i), Scalar(0,0,200), 3);
				}

				resize(image, image, Size(image.cols*0.6, image.rows*0.6));
				imshow("Image", image);
				waitKey();
			}
		}
	}
}

double distanceMouthNose(cv::vector<double> features, Mat image){
	auto mouth = (features[5]+features[7])/2;
	return abs(features[9]-mouth)/double(image.rows);
}

double distanceNoseEye(cv::vector<double> features, Mat image){
	auto eyes = (features[1]+features[3])/2;
	return abs(features[9]-eyes)/double(image.rows);
}

void test(){
	QString filename = "../HeadPoseEstimation/data/bs011/bs011_PR_D_0.png";
	Mat image = imread(filename.toStdString());
	vector<Rect> eyes = Image::detectEyes(image);
	vector<Rect> nose = Image::detectNose(image);
	vector<Rect> mouth = Image::detectMouth(image);

	std::cout << "Eyes " << eyes.size() << std::endl;
	for(int i=0; i<eyes.size(); ++i){
		rectangle(image, eyes.at(i), Scalar(200, 0, 0), 3);
	}

	std::cout << "Nose " << nose.size() << std::endl;
	for(int i=0; i<nose.size(); ++i){
		rectangle(image, nose.at(i), Scalar(0, 200, 0), 3);
	}

	std::cout << "Mouth " << mouth.size() << std::endl;
	for(int i=0; i<mouth.size(); ++i){
		rectangle(image, mouth.at(i), Scalar(0,0,200), 3);
	}

//	PitchTrainer pt = false;
//	vector<double> fts = pt.detectFeatures(filename);
//	for(int i=0; i<fts.size(); i+=2){
//		circle(image, Point(fts[i], fts[i+1]), 5, Scalar(0, 200, 200), 3);
//	}

	vector<double> features(10);
	features[0] = Image::getLeftEye(eyes).x;
	features[1] = Image::getLeftEye(eyes).y+Image::getLeftEye(eyes).height/2;

	features[2] = Image::getRightEye(eyes).br().x;
	features[3] = Image::getRightEye(eyes).y+Image::getRightEye(eyes).height/2;

	features[4] = mouth.at(0).x;
	features[5] = mouth.at(0).y+mouth.at(0).height/2;

	features[6] = mouth.at(0).br().x;
	features[7] = mouth.at(0).br().y-mouth.at(0).height/2;

	features[8] = nose.at(0).x+nose.at(0).width/2;
	features[9] = nose.at(0).y+nose.at(0).height/2;

	std::cout << distanceMouthNose(features, image)/distanceNoseEye(features, image) << std::endl;

	resize(image, image, Size(image.cols*0.6, image.rows*0.6));
	imshow("Image", image);
	waitKey();
}

/* TODO:
	adaptive resize in image
	nose, mouth, eye detectie constraints opleggen
	=> positie van features zou overal bekend moeten zijn
	de pitch detector de pitch laten teruggeven
*/

int main(int argc, char *argv[]) {
//	test();
	//	YawTrainer yt = false;
	//	yt();
	//	crossValidateYaw("positionsYaw");
	//	crossValidateYaw("positionsYawOrig");
	//	crossValidateYaw("positionsYawBetter");

//			PitchTrainer pt = false;
//			pt();

			crossValidatePitch("positionsPitchNew");

//			if(argc > 1){
//				param = QString(argv[1]).toDouble(); //0.0346 is een goed waarde voor diff, voor div 0.14, 0.041 voor divcal
//			}

//			std::cout << param << std::endl;
//			crossValidatePitch("positionsPitchNew");
	return 0;
}
