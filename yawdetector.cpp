#include "yawdetector.h"
#include "image.h"
#include "landmarkreader.h"
#include "yawtrainer.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <algorithm>
#include <random>

#define hybrid 0

YawDetector::YawDetector(QString positionFile) : positionFile(positionFile){
	positions = deserialize();
}

YawDetector& YawDetector::retainDir(QString retainDir){
	auto keys = positions.keys();
	for(int i=0; i<keys.count(); ++i){
		if(keys.at(i).contains(retainDir)){
			positions.remove(keys.at(i));
		}
	}
	return *this;
}

size_t YawDetector::size(QPair<long, long> pair){
	if(pair.first == 0 && pair.second == 0){
		return 0;
	} else if (pair.first == 0 || pair.second == 0){
		return 1;
	} else {
		return 2;
	}
}

long YawDetector::operator()(QString image){
	auto pair1 = position(image);
	size_t eyes1Size = size(pair1);
	long position1 = pair1.first + pair1.second;

	QMultiMap<long, QString> scores;
	auto images = positions.keys();
	foreach(QString image, images){
		auto pair2 = positions.value(image);
		long score = 0;

		if(eyes1Size != size(pair2)){
			score = ONE_MILLION;
		} else {
			long position2 = pair2.first + pair2.second;
			if(eyes1Size == 0){
				score = 0;
			} else if(eyes1Size == 1){
				if((pair1.first == 0 && pair2.first == 0) || (pair1.second == 0 && pair2.second == 0)){
					score = (position1 - position2);
				} else {
					score = ONE_MILLION;
				}
			} else {
				score = (position1 - position2);
			}
		}

		scores.insertMulti(std::abs(score), image);
	}

	return getYawOfBest(scores);
}

QMap<QString, QPair<long, long> > YawDetector::deserialize(){
	QMap<QString, QPair<long, long> > result;

	QFile file(positionFile);
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

long YawDetector::getYawOfBest(QMultiMap<long, QString> scores, int number){
	QList<long> keys = scores.keys();
	qSort(keys.begin(), keys.end());
	QStringList hits = scores.values(keys.at(number));
	QMap<long, long> yaws;
	foreach(QString hit, hits){
		++yaws[yaw(hit)];
	}
	int max = 0;
	int detectedYaw = 0;
	for(int i=0; i<yaws.keys().count(); ++i){
		long value = yaws[yaws.keys().at(i)] ;
		if(value > max){
			max = value;
			detectedYaw = yaws.keys().at(i);
		}
	}
	return detectedYaw;
}

int YawDetector::yaw(QString filename){
	int rot;
	QFileInfo info(filename);
	filename = info.fileName();
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

QPair<long, long> YawDetector::position(QString filename){
#if hybrid
	QString tmp = positionFile;
	positionFile = "positionsYaw";
#endif //hybrid
	QFileInfo info1(filename);
	auto allPositions = deserialize();
	auto keys = allPositions.keys();
	for(int i=0; i<keys.count(); ++i){
		QFileInfo info2(keys.at(i));
		if(info1.fileName() == info2.fileName()){
#if hybrid
			positionFile = tmp;
#endif //hybrid
			return allPositions[keys.at(i)];
		}
	}
#if hybrid
	positionFile = tmp;
#endif //hybrid

	YawTrainer yt = false;
	return yt.detectPositions(filename);
}
