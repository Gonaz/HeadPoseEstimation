#include "pitchdetector.h"
#include "image.h"
#include "landmarkreader.h"
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <iostream> //TODO debug

using namespace cv;

PitchDetector::PitchDetector(bool train) {
	if(train){
		positions = calculateRelativePositions();
		serialize(positions);
	} else {
		positions = deserialize();
	}
}

PitchDetector::PitchDetector(QString retainDir){
	positions = deserialize();
	auto keys = positions.keys();
	for(int i=0; i<keys.count(); ++i){
		if(keys.at(i).contains(retainDir)){
			positions.remove(keys.at(i));
		}
	}
}

cv::vector<double> PitchDetector::readFeatures(QString filename){
	filename.replace(".png", ".lm2");
	LandMarkReader lmr = LandMarkReader(filename);
	vector<double> features(10);

	try{
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
	} catch(std::exception e) {
//		std::cerr << "Error " << e.what() << std::endl; //TODO dit mag misschien weg
	}

	return features;
}

cv::vector<double> PitchDetector::detectFeatures(Mat image){
	vector<Rect> eyes = Image::detectEyes(image);
	vector<Rect> mouth = Image::detectMouth(image);
	vector<Rect> nose = Image::detectNose(image);
	vector<double> features(10);

	try{
		features[0] = Image::getLeftEye(eyes).x;
		features[1] = Image::getLeftEye(eyes).y+Image::getLeftEye(eyes).size().height/2;

		features[2] = Image::getRightEye(eyes).br().x;
		features[3] = Image::getRightEye(eyes).y+Image::getRightEye(eyes).size().height/2;

		features[4] = mouth.at(0).x;
		features[5] = mouth.at(0).y+mouth.at(0).size().height/2;

		features[6] = mouth.at(0).br().x;
		features[7] = mouth.at(0).br().y-mouth.at(0).size().height/2;

		features[8] = nose.at(0).x+nose.at(0).size().width/2;
		features[9] = nose.at(0).y+nose.at(0).size().height/2;
	} catch(std::exception e) {
//		std::cerr << "Error " << e.what() << std::endl; //TODO dit mag misschien weg
	}

	return features;
}

double PitchDetector::distanceMouthNose(cv::vector<double> features, Mat image){
	auto mouth = (features[5]+features[7])/2;
	return abs(features[9]-mouth)/double(image.rows);
}

double PitchDetector::distanceNoseEye(cv::vector<double> features, Mat image){
	auto eyes = (features[1]+features[3])/2;
	return abs(features[9]-eyes)/double(image.rows);
}

QMap<QString, QPair<long, double> > PitchDetector::calculateRelativePositions(){
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
			vector<double> features = readFeatures(imagePath);
			//vector<double> features = detectFeatures(im);
			long realPitch = PitchDetector::pitch(imagePath);

//			double diff = distanceMouthNose(features, im)-distanceNoseEye(features, im);
			double div = distanceMouthNose(features, im)/distanceNoseEye(features, im);

			std::cout << "Detect " << image.toStdString();
			std::cout << "\t" << div << std::endl;
			result[imagePath]= qMakePair(realPitch, div);
		}
	}
}

return result;
}

bool PitchDetector::containsTies(QVector<long> vec){
	int max = 0;

	foreach(long v, vec){
		if(v>max){
			max = v;
		}
	}

	return (vec.count(max) > 1) ? true : false;
}

double PitchDetector::diffFromFile(QString filename){
	QFileInfo info1(filename);
	auto allPositions = deserialize();
	auto keys = allPositions.keys();
	for(int i=0; i<keys.count(); ++i){
		QFileInfo info2(keys.at(i));
		if(info1.fileName() == info2.fileName()){
			return allPositions[keys.at(i)].second;
		}
	}
}

QVector<long> PitchDetector::detectPitch(QString filename, double fuzziness){
	QMap<long, QVector<float> > result;

	auto ks = positions.keys();
	for(int i=0; i<ks.count(); ++i){
		auto pair = positions.value(ks.at(i));
		result[pair.first].push_back(pair.second);
	}

//	Mat im = imread(filename.toStdString());
//	auto features = detectFeatures(im);
//	double diff = distanceMouthNose(features, im)-distanceNoseEye(features, im);
	double diff = diffFromFile(filename);

	auto keys = result.keys();
	QVector<long> support(keys.count(), 0);
	for(int i=0; i<keys.count(); ++i){
		auto values = result.value(keys.at(i));
		for(int j=0; j<values.count(); ++j){
			if(isnan(values.at(j))){
				values.replace(j, 0); //TODO temporary workaround
			}
			if(isnan(diff)){
				diff = 0; //TODO temporary workaround
			}

			double a = std::abs(values.at(j) - diff);
			if(a < fuzziness){
				support.replace(i, support.at(i)+1);
			}
		}
	}

	return support;
}

long PitchDetector::operator()(QString filename, double fuzziness){
	auto support = detectPitch(filename, fuzziness);
	int counter = 0;
	while(containsTies(support) && fuzziness > 0){
//		std::cout << "Tie" << std::endl; //TODO dit mag later weg
		++counter;
		fuzziness -= 0.0002;
		support = detectPitch(filename, fuzziness);
	}
	while(containsTies(support)){
//		std::cout << "Tie" << std::endl; //TODO dit mag later weg
		++counter;
		fuzziness += 0.002;
		support = detectPitch(filename, fuzziness);
	}

	QVector<long> keys;
	keys << -2 << -1 << 0 << 1 << 2;
	for(int i=0; i<keys.count(); ++i){
		std::cout << keys.at(i) << " -> " << support.at(i) << std::endl;
	}

	//TODO return the pitch
}

long PitchDetector::pitch(QString filename){
	filename = filename.split("/").last(); //TODO kan dit niet mooier met fileInfo?
	if(filename.contains("PR")){
		int index = filename.lastIndexOf("PR")+3;
		filename = filename.mid(index, filename.length()-index-6);
		if(filename == "D"){
			return -2;
		} else if(filename == "SD"){
			return -1;
		} else if(filename == "SU"){
			return 1;
		} else if (filename == "U"){
			return 2;
		}
	}

	return 0;
}

void PitchDetector::serialize(QMap<QString, QPair<long, double> > result){
	QFile file("positionsPitchOrig");
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

QMap<QString, QPair<long, double> > PitchDetector::deserialize(){
	QMap<QString, QPair<long, double> > result;

	QFile file("positionsPitchOrig");
	if(file.open(QIODevice::ReadOnly)){
		QTextStream stream(&file);

		while(!stream.atEnd()){
			QString key;
			QPair<long, double> value;
			key = stream.readLine();
			value.first = stream.readLine().toLong();
			value.second = stream.readLine().toDouble();

			result.insert(key, value);
		}
	}

	file.close();
	return result;
}
