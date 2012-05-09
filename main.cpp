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
double param = 0.14;

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
				} else {
					++correct;
					++correctDir;
//					std::cout << image.toStdString() << "\t" << realPitch << "\t" << detectedPitch << std::endl;
				}
			}
		}
//				std::cout << imageDir.toStdString() << " -> " << correctDir << "/" << (correctDir + wrongDir) << std::endl;
	}
	std::cout << correct << "/" << (correct+wrong) << std::endl;
	std::cout << "Average absolute error " << (error/double(correct+wrong)) << std::endl;
}

Point relative(Point original, Mat image){
	return Point(original.x/double(image.cols)*100, original.y/double(image.rows)*100);
}

void test2(){
	QString filename = "../HeadPoseEstimation/data/bs002/bs002_PR_D_0.png";
	Mat image = imread(filename.toStdString());
	QString file = "/usr/local/share/OpenCV/haarcascades/haarcascade_mcs_mouth.xml";
	int amount = 1;

	cv::vector<cv::Rect> rectangles;
	cv::CascadeClassifier cc(file.toStdString());
	cc.detectMultiScale(image, rectangles);
	int param = 5;
	while(rectangles.size() > amount){
		cc.detectMultiScale(image, rectangles, 1.1, param);
		param += 5;

		std::cout << rectangles.size() << std::endl;
		Mat image2;
		image.copyTo(image2);
		for(int i=0; i<rectangles.size(); ++i){
			if((rectangles.at(i).br().y)/double(image.rows)*100 < 50){
				rectangle(image2, rectangles.at(i), Scalar(0, 0, 200), 3);
				rectangles.erase(rectangles.begin()+i);
				--i;
			}
		}


		for(int i=0; i<rectangles.size(); ++i){
			rectangle(image2, rectangles.at(i), Scalar(0, 200, 0), 3);
		}
		resize(image2, image2, Size(image.cols*0.6, image.rows*0.6));
		imshow("Mouth", image2);
		waitKey();
	}
	std::cout << "Finished" << std::endl;
}

void test(){
	QString filename = "../HeadPoseEstimation/data/bs002/bs002_PR_D_0.png";
	Mat image = imread(filename.toStdString());
	vector<Rect> eyes = Image::detectEyes(image);
	vector<Rect> mouth = Image::detectMouth(image);
	std::cout << "Amount of mouths " << mouth.size() << std::endl;
	std::cout << mouth.at(0).x << "|" << mouth.at(0).width << "|" << mouth.at(0).y << "|" << mouth.at(0).height << std::endl;
	rectangle(image, mouth.at(0), Scalar(0, 200, 0), 3);
	resize(image, image, Size(image.cols*0.6, image.rows*0.6));
	imshow("mouth", image);
	waitKey();
	vector<Rect> nose = Image::detectNose(image);
	vector<double> features(10);

	try{
		features[0] = Image::getLeftEye(eyes).x;
		features[1] = Image::getLeftEye(eyes).y+Image::getLeftEye(eyes).size().height/2;

		std::cout << "Hoi" << std::endl;

		features[2] = Image::getRightEye(eyes).br().x;
		features[3] = Image::getRightEye(eyes).y+Image::getRightEye(eyes).size().height/2;

		std::cout << "Hoi" << std::endl;

		features[4] = mouth.at(0).x;
		features[5] = mouth.at(0).y+mouth.at(0).size().height/2;

		std::cout << "Hoi" << std::endl;

		features[6] = mouth.at(0).br().x;
		features[7] = mouth.at(0).br().y-mouth.at(0).size().height/2;

		std::cout << "Hoi" << std::endl;

		features[8] = nose.at(0).x+nose.at(0).size().width/2;
		features[9] = nose.at(0).y+nose.at(0).size().height/2;
	} catch(std::exception e) {
		std::cerr << "Error " << e.what() << std::endl; //TODO dit mag misschien weg
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
	//	YawTrainer yt = false;
	//	yt();
	//	crossValidateYaw("positionsYaw");
	//	crossValidateYaw("positionsYawOrig");
	//	crossValidateYaw("positionsYawBetter");

//		PitchTrainer pt = false;
//		pt();

//		crossValidatePitch("positionsPitch");

		if(argc > 1){
			param = QString(argv[1]).toDouble(); //0.0346 is een goed waarde voor diff, voor div 0.14
		}

		std::cout << param << std::endl;
		crossValidatePitch("positionsPitchDivCal");
	return 0;
}
