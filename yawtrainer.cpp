#include "yawtrainer.h"
#include "image.h"
#include "landmarkreader.h"
#include <QDir>
#include <QTextStream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <functional>

//Logical means that the results are logical interpretable. But the accuracy is a bit worse.
//When logical is set, the position of an eye with one landmark is the position of that landmark.
//When logical is unset, the position of an eye with one landmark is half the position of that landmark.
#define logical 0

//Sorted means that the position of the eyes will be sorted.
//When sorted is set, first the biggest position is serialized and then the smallest position.
//When sorted is unset, first the position of the right eye is serialized and then the position of the left eye.
#define sorted 0

using namespace cv;

YawTrainer::YawTrainer(bool landmarks)  {
	using namespace std::placeholders;
	if(landmarks){
		positionFile = "positionsYawOrig";
		positions = std::bind(&YawTrainer::readPositions, this, _1);
	} else {
		positionFile = "positionsYaw";
		positions = std::bind(&YawTrainer::detectPositions, this, _1);
	}
}

void YawTrainer::operator ()(){
	auto positions = calculateRelativePositions();
	serialize(positions);
}

QMap<QString, QPair<long, long> > YawTrainer::calculateRelativePositions() {
	QMap<QString, QPair<long, long> > result;
	QStringList subdirs = QDir("../HeadPoseEstimation/data/").entryList();
	auto last = std::remove_if(subdirs.begin(), subdirs.end(), [](QString s){return !s.startsWith("bs");});

	for(auto elem=subdirs.begin(); elem != last; ++elem){
		QDir imageDir("../HeadPoseEstimation/data/" + *(elem));
		QStringList images = imageDir.entryList(QStringList("*.png"));
		foreach(QString image, images){
			QString imagePath = "../HeadPoseEstimation/data/" + *(elem) + "/" + image;
			std::cout << "Detect " << imagePath.toStdString() << std::endl;
			result.insert(imagePath, positions(imagePath));
		}
	}

	return result;
}

QPair<long, long> YawTrainer::detectPositions(QString imagePath){
	Mat image = imread(imagePath.toStdString());
	vector<Rect> eyes = Image::detectEyes(image);
	if(eyes.size() == 0){
		return qMakePair(long(0), long(0));
	} else if(eyes.size() == 1){
		long value = Image::getRelativePositionEye(image, eyes.at(0));
#if sorted
		return qMakePair(value, long(0));
#else
		if(value < 50){
			return qMakePair(value, long(0));
		} else {
			return qMakePair(long(0), value);
		}
#endif //sorted
	} else {
		if(eyes.at(0).x > eyes.at(1).x){
			return qMakePair(Image::getRelativePositionEye(image, eyes.at(0)), Image::getRelativePositionEye(image, eyes.at(1)));
		} else {
			return qMakePair(Image::getRelativePositionEye(image, eyes.at(1)), Image::getRelativePositionEye(image, eyes.at(0)));
		}
	}
}

QPair<long, long> YawTrainer::readPositions(QString imagePath){
	Mat image = imread(imagePath.toStdString());
	QString landmarksPath(imagePath);
	landmarksPath.replace(".png", ".lm2");
	LandMarkReader lmr = LandMarkReader(landmarksPath);
	long leftOuter = lmr.leftEyeCorner().first;
	long leftInner = lmr.innerLeftEyeCorner().first;
	long rightOuter = lmr.rightEyeCorner().first;
	long rightInner = lmr.innerRightEyeCorner().first;

#if logical
	long left = 0;
	long right = 0;

	if(leftOuter == 0 && leftInner != 0){
		left = leftInner/double(image.cols)*100;
	} else if (leftOuter != 0 && leftInner == 0){
		left = leftOuter/double(image.cols)*100;
	} else {
		left = (leftOuter + leftInner)/2/double(image.cols)*100;
	}

	if(rightOuter == 0 && rightInner != 0){
		right = rightInner/double(image.cols)*100;
	} else if(rightOuter != 0 && rightInner == 0) {
		right = rightOuter/double(image.cols)*100;
	} else {
		right = (rightOuter + rightInner)/2/double(image.cols)*100;
	}
#else //logical
	long left = (leftOuter + leftInner)/2/double(image.cols)*100;
	long right = (rightOuter + rightInner)/2/double(image.cols)*100;
#endif //logical

#if sorted
	if(left > right){
		return qMakePair(left, right);
	} else {
		return qMakePair(right, left);
	}
#else //sorted
	return qMakePair(right, left);
#endif //sorted
}

void YawTrainer::serialize(QMap<QString, QPair<long, long> > positions){
	QFile file(positionFile);
	if(file.open(QIODevice::WriteOnly)){
		QTextStream stream(&file);

		for(int i=0; i<positions.count(); ++i){
			auto key = positions.keys().at(i);
			auto value = positions.value(key);

			stream << key << "\n" << value.first << "\n" << value.second << "\n";
		}
	}

	file.close();
}
