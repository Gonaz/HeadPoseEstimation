#include "yawdetector.h"
#include "image.h"
#include <QDir>
#include <QTextStream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <algorithm>
#include <iostream> //TODO debug

using namespace cv;

YawDetector::YawDetector(bool train){
	if(train){
		positions = calculateRelativePositions();
		serialize(positions);
	} else {
		positions = deserialize();
	}
}

YawDetector::YawDetector(QString retainDir){
	positions = deserialize();
	auto keys = positions.keys();
	for(int i=0; i<keys.count(); ++i){
		if(keys.at(i).contains(retainDir)){
			positions.remove(keys.at(i));
		}
	}
}

long YawDetector::operator()(QString image){
	Mat image1 = imread(image.toStdString());
	vector<Rect> eyes1 = Image::detectEyes(image1);
	long position1 = 0;
	for(size_t i=0; i<eyes1.size(); ++i){
		position1 += Image::getRelativePositionEye(image1, eyes1.at(i));
	}
	if(eyes1.size() > 0){
		position1 /= eyes1.size();
	}
	std::cout << "Aantal ogen " << eyes1.size() << std::endl;

	QMultiMap<long, QString> scores;
	auto images = positions.keys();
	foreach(QString image, images){
		auto pair = positions.value(image);
		long score = 0;

		if(eyes1.size() != size(pair) || eyes1.size() == 0 || size(pair) == 0){
			score = ONE_MILLION;
		} else {
			long position2 = pair.first;
			if(size(pair) == 2){
				position2 += pair.second;
			}
			if(size(pair) > 0){
				position2 /= size(pair);
			}

			score = (position1 - position2);
		}

		scores.insertMulti(abs(score), image);
	}

	if(scores.values().contains(image)){
		auto key = scores.key(image);
		scores.remove(key);
	}

	return yaw(getBest(scores).second);
}

QMap<QString, QPair<long, long> > YawDetector::calculateRelativePositions() {
	//TODO this method can be made more general
	QMap<QString, QPair<long, long> > result;
	QStringList subdirs = QDir("../HeadPoseEstimation/data/").entryList();
	auto last = std::remove_if(subdirs.begin(), subdirs.end(), [](QString s){return !s.startsWith("bs");});

	for(auto elem=subdirs.begin(); elem != last; ++elem){
		QDir imageDir("../HeadPoseEstimation/data/" + *(elem));
		QStringList images = imageDir.entryList(QStringList("*.png"));
		foreach(QString image, images){
			QString imagePath = "../HeadPoseEstimation/data/" + *(elem) + "/" + image;
			Mat image = imread(imagePath.toStdString());
			vector<Rect> eyes = Image::detectEyes(image);
			if(eyes.size() == 0){
				result.insert(imagePath, qMakePair(-1l, -1l));
			} else if(eyes.size() == 1){
				result.insert(imagePath, qMakePair(Image::getRelativePositionEye(image, eyes.at(0)), -1l));
			} else {
				result.insert(imagePath, qMakePair(Image::getRelativePositionEye(image, eyes.at(0)), Image::getRelativePositionEye(image, eyes.at(1))));
			}
		}
	}

	return result;
}

void YawDetector::serialize(QMap<QString, QPair<long, long> > positions){
	QFile file("positionsYaw");
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

QMap<QString, QPair<long, long> > YawDetector::deserialize(){
	QMap<QString, QPair<long, long> > result;

	QFile file("positionsYaw");
	if(file.open(QIODevice::ReadOnly)){
		QTextStream stream(&file);

		while(!stream.atEnd()){
			QString key;
			QPair<long, long> value;
			key = stream.readLine();
			value.first = stream.readLine().toLong();
			value.second = stream.readLine().toLong();

			result.insert(key, value);
		}
	}

	file.close();
	return result;
}

QPair<long, QString> YawDetector::getBest(QMultiMap<long, QString> scores, int number){
	QList<long> keys = scores.keys();
	qSort(keys.begin(), keys.end()); //TODO betere sorteer methode?
	return qMakePair(keys.at(number), scores.value(keys.at(number)));
}

size_t YawDetector::size(QPair<long, long> pair){
	if(pair.first == -1){
		return 0;
	} else if(pair.second == -1){
		return 1;
	}
	return 2;
}

int YawDetector::yaw(QString filename){
	int rot;
	filename = filename.split("/").last(); //TODO kan dit niet mooier met fileInfo?
	if(filename.contains("YR")){
		int index = filename.lastIndexOf("YR")+3;
		rot = filename.mid(index+1, 2).toInt();
		if(filename.at(index) == 'L'){
			rot *= -1;
		}
	} else {
		return 0;
	}

	return rot;
}
