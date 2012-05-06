#include "yawtrainer.h"
#include "image.h"
#include "landmarkreader.h"
#include <QDir>
#include <QTextStream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <functional>

using namespace cv;

YawTrainer::YawTrainer(bool landmarks) : landmarks(landmarks) {
	using namespace std::placeholders;
	if(landmarks){
		positionFile = "positionsYawOrig";
		positions = std::bind(&YawTrainer::readPositions, this, _1);
	} else {
		positionFile = "positionsYawBetter";
		positions = std::bind(&YawTrainer::detectBetterPositions, this, _1);
	}
}

void YawTrainer::operator ()(){
	auto positions = calculateRelativePositions();
	serialize(positions);
}

QMap<QString, QPair<long, long> > YawTrainer::calculateRelativePositions() {
	//TODO this method can be made more general
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
//---------------------------------------------------------------------------
		return qMakePair(Image::getRelativePositionEye(image, eyes.at(0)), long(0));
//		long position = Image::getRelativePositionEye(image, eyes.at(0));
//		if(position < 50){
//			return qMakePair(position, long(0));
//		} else {
//			return qMakePair(long(0), position);
//		}
	} else {
//---------------------------------------------------------------------------
		if(eyes.at(0).x > eyes.at(1).x){
			return qMakePair(Image::getRelativePositionEye(image, eyes.at(0)), Image::getRelativePositionEye(image, eyes.at(1)));
		} else {
			return qMakePair(Image::getRelativePositionEye(image, eyes.at(1)), Image::getRelativePositionEye(image, eyes.at(0)));
		}
//		return qMakePair(Image::getRelativePositionEye(image, eyes.at(0)), Image::getRelativePositionEye(image, eyes.at(1)));
	}
}

int dis(Point p1, Point p2){
	return std::abs(std::sqrt(std::pow(p1.x - p2.x, 2) + std::pow(p1.y - p2.y, 2)));
}

Point nearestFeature(Mat im, Point p, int maxDistance){
	Point best;
	int minDistance = maxDistance;

	double scale = 0.2;
	double iScale = 1/scale;
	Mat image;
	cv::resize(im, image, cv::Size(im.cols*scale, im.rows*scale));

	for(int lowerThreshold=44; lowerThreshold<=56; ++lowerThreshold){
		int blurSize = 15;

		Mat gray;
		cvtColor(image, gray, CV_BGR2GRAY);
		Mat grayThresholded;
		threshold(gray, grayThresholded, lowerThreshold, 255, THRESH_BINARY);
		GaussianBlur(grayThresholded, grayThresholded, Size(blurSize, blurSize), 0);

		Mat out, mask;
		const int maxCorners = 100;
		double qualityLevel = 0.01;
		double minimumDistance = 10;
		int blockSize = 3;
		bool useHarrisDetector = false;
		double k = 0.04;
		cv::goodFeaturesToTrack(grayThresholded, out, maxCorners, qualityLevel, minimumDistance, mask, blockSize, useHarrisDetector, k);

		Point point;
		for(int i=0; i<out.rows; ++i){
			int x = out.row(i).at<float>(0,0)*iScale;
			int y = out.row(i).at<float>(0,1)*iScale;
			point = Point(x, y);
			int distance = dis(p, point);
			if(distance < maxDistance){
				if(distance < minDistance){
					minDistance = distance;
					best = point;
				}
			}
		}
		maxDistance = minDistance;
	}

	return best;
}

QPair<long, long> YawTrainer::detectBetterPositions(QString imagePath){
	Mat image = imread(imagePath.toStdString());
	vector<Rect> eyes = Image::detectEyes(image);
	Point leftOuter, leftInner, rightOuter, rightInner;
	Point featureLeftOuter, featureLeftInner, featureRightOuter, featureRightInner;
	if(eyes.size() == 2){
		if(eyes.at(0).tl().x < eyes.at(1).tl().x){
			auto tmp = eyes.at(0);
			eyes.at(0) = eyes.at(1);
			eyes.at(1) = tmp;
		}
		Point tl = eyes.at(0).tl();
		Point br = eyes.at(0).br();
		leftOuter = Point(br.x, (tl.y + br.y)/2);
		leftInner = Point(tl.x, (tl.y + br.y)/2);

		tl = eyes.at(1).tl();
		br = eyes.at(1).br();
		rightOuter = Point(tl.x, (tl.y + br.y)/2);
		rightInner = Point(br.x, (tl.y + br.y)/2);
		int maxDistance = std::min(std::abs(leftOuter.x - leftInner.x)/3, std::abs(rightOuter.x - rightInner.x)/3);
		featureLeftOuter = nearestFeature(image, leftOuter, maxDistance);
		featureLeftInner = nearestFeature(image, leftInner, maxDistance);
		featureRightOuter = nearestFeature(image, rightOuter, maxDistance);
		featureRightInner = nearestFeature(image, rightInner, maxDistance);
	} else if(eyes.size() == 1){
		Point tl = eyes.at(0).tl();
		Point br = eyes.at(0).br();
		leftOuter = Point(br.x, (tl.y + br.y)/2);
		leftInner = Point(tl.x, (tl.y + br.y)/2);

		int maxDistance = std::abs(leftOuter.x - leftInner.x)/3;
		featureLeftOuter = nearestFeature(image, leftOuter, maxDistance);
		featureLeftInner = nearestFeature(image, leftInner, maxDistance);
	} else {
		std::cout << "Hoi-------------------------------------------" << std::endl;
	}

	long leftOuterX = featureLeftOuter.x;
	long leftInnerX = featureLeftInner.x;
	long rightOuterX = featureRightOuter.x;
	long rightInnerX = featureRightInner.x;

	long left = left = (leftOuterX + leftInnerX)/2/double(image.cols)*100;
	long right = (rightOuterX + rightInnerX)/2/double(image.cols)*100;
	if(left > right){
		return qMakePair(left, right);
	} else {
		return qMakePair(right, left);
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

	long left = left = (leftOuter + leftInner)/2/double(image.cols)*100;
	long right = (rightOuter + rightInner)/2/double(image.cols)*100;

//	long left = 0;
//	long right = 0;

//	if(leftOuter == 0 && leftInner != 0){
//		left = leftInner/double(image.cols)*100;
//	} else if (leftOuter != 0 && leftInner == 0){
//		left = leftOuter/double(image.cols)*100;
//	} else {
//		left = (leftOuter + leftInner)/2/double(image.cols)*100;
//	}

//	if(rightOuter == 0 && rightInner != 0){
//		right = rightInner/double(image.cols)*100;
//	} else if(rightOuter != 0 && rightInner == 0) {
//		right = rightOuter/double(image.cols)*100;
//	} else {
//		right = (rightOuter + rightInner)/2/double(image.cols)*100;
//	}

//---------------------------------------------------------------------------
	if(left > right){
		return qMakePair(left, right);
	} else {
		return qMakePair(right, left);
	}
//	return qMakePair(right, left);
}

void YawTrainer::serialize(QMap<QString, QPair<long, long> > positions){
	QFile file(positionFile);
	if(file.open(QIODevice::WriteOnly)){
		QTextStream stream(&file);

		for(int i=0; i<positions.count(); ++i){
			auto key = positions.keys().at(i);
			auto value = positions.value(key);

//			if(value.first > value.second){
				stream << key << "\n" << value.first << "\n" << value.second << "\n";
//			} else {
//				stream << key << "\n" << value.second << "\n" << value.first << "\n";
//			}
		}
	}

	file.close();
}
