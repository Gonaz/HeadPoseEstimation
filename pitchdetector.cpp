#include "pitchdetector.h"
#include "image.h"
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

PitchDetector::PitchDetector(QString retainDir){ //TODO write this piece of code
//	positions = deserialize();
//	auto keys = positions.keys();
//	for(int i=0; i<keys.count(); ++i){
//		if(keys.at(i).contains(retainDir)){
//			positions.remove(keys.at(i));
//		}
//	}
}

vector<float> PitchDetector::detectFeatures(Mat image){
	vector<Rect> eyes = Image::detectEyes(image);
	vector<Rect> mouth = Image::detectMouth(image);
	vector<Rect> nose = Image::detectNose(image);
	vector<float> features(10);

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
		std::cerr << "Error " << e.what() << std::endl;
	}

	return features;
}

double PitchDetector::distanceMouthNose(vector<float> features, Mat image){
	auto mouth = (features[5]+features[7])/2;
	return abs(features[9]-mouth)/double(image.rows);
}

double PitchDetector::distanceNoseEye(vector<float> features, Mat image){
	auto eyes = (features[1]+features[3])/2;
	return abs(features[9]-eyes)/double(image.rows);
}

QMap<long, QVector<float> > PitchDetector::calculateRelativePositions(){
	QMap<long, QVector<float> > result;
	QStringList subdirs = QDir("../HeadPoseEstimation/data/").entryList();
	auto last = std::remove_if(subdirs.begin(), subdirs.end(), [](QString s){return !s.startsWith("bs");});

for(auto elem=subdirs.begin(); elem != last; ++elem){
	QDir imageDir("../HeadPoseEstimation/data/" + *(elem));
	QStringList images = imageDir.entryList(QStringList("*.png"));
	foreach(QString image, images){
		QString imagePath = "../HeadPoseEstimation/data/" + *(elem) + "/" + image;
		if(!image.contains("YR")){
			Mat im = imread(imagePath.toStdString());
			vector<float> features = detectFeatures(im);
			long realPitch = PitchDetector::pitch(imagePath);

			double diff = distanceMouthNose(features, im)-distanceNoseEye(features, im);

			std::cout << "Detect " << image.toStdString();
			std::cout << "\t" << diff << std::endl;
			result[realPitch].push_back(diff);
		}
	}
}

return result;
}



bool containsTies(QVector<long> vec){
	int max = 0;

	foreach(long v, vec){
		if(v>max){
			max = v;
		}
	}
	if(vec.count(max) > 1){
		return true;
	}
	return false;
}

QVector<long> PitchDetector::test(QString filename, float fuzziness){
	QMap<long, QVector<float> > result;

	QFile file("positionsPitch");
	if(file.open(QIODevice::ReadOnly)){
		QTextStream stream(&file);

		while(!stream.atEnd()){
			long key;
			float value;
			key = stream.readLine().toLong();
			value = stream.readLine().toFloat();

			result[key].push_back(value);
		}
	}

	file.close();

	Mat im = imread(filename.toStdString());
	auto features = detectFeatures(im);
	double diff = distanceMouthNose(features, im)-distanceNoseEye(features, im);

	auto keys = result.keys();
	QVector<long> support(keys.count(), 0);
	for(int i=0; i<keys.count(); ++i){
		auto values = result.value(keys.at(i));
		for(int j=0; j<values.count(); ++j){
			double a = std::abs(values.at(j) - diff);
			if(a < fuzziness){
				support.replace(i, support.at(i)+1);
			}
		}
	}

	//	for(int i=0; i<keys.count(); ++i){
	//		std::cout << keys.at(i) << " -> " << support.at(i) << std::endl;
	//	}

	return support;
}

long PitchDetector::operator()(QString filename, float fuzziness){
	auto support = test(filename, fuzziness);
	int counter = 0;
	while(containsTies(support) && counter < 10){
		std::cout << "Tie" << std::endl;
		++counter;
		fuzziness -= 0.0001;
		support = test(filename, fuzziness);
	}

	QVector<long> keys;
	keys << -2 << -1 << 0 << 1 << 2;
	for(int i=0; i<keys.count(); ++i){
		std::cout << keys.at(i) << " -> " << support.at(i) << std::endl;
	}
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

void PitchDetector::serialize(QMap<long, QVector<float> > result){
	QFile file("positionsPitch");
	if(file.open(QIODevice::WriteOnly)){
		QTextStream stream(&file);

		for(int i=0; i<result.count(); ++i){
			auto key = result.keys().at(i);
			auto values = result.value(key);
			foreach(float value, values){
				stream << key << "\n" << value << "\n";
			}
		}
	}

	file.close();
}

QMap<long, QVector<float> > PitchDetector::deserialize(){
	//TODO write this piece of code
}
