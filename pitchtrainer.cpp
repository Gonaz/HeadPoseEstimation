#include "pitchtrainer.h"
#include "landmarkreader.h"
#include "image.h"
#include "pitchdetector.h"
#include <QStringList>
#include <QDir>
#include <QTextStream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream> //TODO: debug

using namespace cv;

PitchTrainer::PitchTrainer(bool landmarks) {
	using namespace std::placeholders;
	if(landmarks){
		positionFile = "positionsPitchOrig";
		features = std::bind(&PitchTrainer::readFeatures, this, _1);
	} else {
		positionFile = "positionsPitch";
		features = std::bind(&PitchTrainer::detectFeatures, this, _1);
	}
}

void PitchTrainer::operator ()(){
	auto positions = calculateRelativePositions();
	serialize(positions);
}

vector<double> PitchTrainer::readFeatures(QString filename){
	filename.replace(".png", ".lm2");
	LandMarkReader lmr = LandMarkReader(filename);
	vector<double> features(10);

	features[0] = lmr.leftEyeCorner().first;
	features[1] = lmr.leftEyeCorner().second;

	features[2] = lmr.rightEyeCorner().first;
	features[3] = lmr.rightEyeCorner().second;

	features[4] = lmr.mouthLeftCorner().first;
	features[5] = lmr.mouthLeftCorner().second;

	features[6] = lmr.mouthRightCorner().first;
	features[7] = lmr.mouthLeftCorner().second;

	features[8] = lmr.nose().first;
	features[9] = lmr.nose().second;

	return features;
}

vector<double> PitchTrainer::detectFeatures(QString filename){
	Mat image = imread(filename.toStdString());
	vector<Rect> eyes = Image::detectEyes(image);
	vector<Rect> mouth = Image::detectMouth(image);
	vector<Rect> nose = Image::detectNose(image);
	vector<double> features(10);

	try{
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
	} catch(std::exception const& e) {
		return features;
	}

	return features;
}

double PitchTrainer::distanceMouthNose(cv::vector<double> features, Mat image){
	auto mouth = (features[5]+features[7])/2;
	return abs(features[9]-mouth)/double(image.rows);
}

double PitchTrainer::distanceNoseEye(cv::vector<double> features, Mat image){
	auto eyes = (features[1]+features[3])/2;
	return abs(features[9]-eyes)/double(image.rows);
}

QMap<QString, QPair<long, double> > PitchTrainer::calculateRelativePositions(){
	QMap<QString, QPair<long, double> > result;
	QStringList subdirs = QDir("../HeadPoseEstimation/data/").entryList();
	auto last = std::remove_if(subdirs.begin(), subdirs.end(), [](QString s){return !s.startsWith("bs");});

	for(auto elem=subdirs.begin(); elem != last; ++elem){
		QDir imageDir("../HeadPoseEstimation/data/" + *(elem));
		QStringList images = imageDir.entryList(QStringList("*.png"));
		foreach(QString image, images){
			QString imagePath = "../HeadPoseEstimation/data/" + *(elem) + "/" + image;
			if(!image.contains("YR")){
				Mat im = imread(imagePath.toStdString());
				vector<double> fts = features(imagePath);
				long realPitch = PitchDetector::pitch(imagePath);

				double div = distanceMouthNose(fts, im)/distanceNoseEye(fts, im);

				std::cout << "Detect " << image.toStdString();
				std::cout << "\t" << div << std::endl;
				result[imagePath]= qMakePair(realPitch, div);
			}
		}
	}

	return result;
}

void PitchTrainer::serialize(QMap<QString, QPair<long, double> > positions){
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
