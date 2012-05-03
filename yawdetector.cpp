#include "yawdetector.h"
#include "image.h"
#include "landmarkreader.h"
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

size_t YawDetector::size(QPair<long, long> pair){
	if(pair.first == 0){
		return 0;
	} else if(pair.second == 0){
		return 1;
	}
	return 2;
}

QVector<long> YawDetector::detectYaw(QString filename, double fuzziness){
	QMap<long, QVector<long> > result;

	auto ks = positions.keys();
	for(int i=0; i<ks.count(); ++i){
		auto pair = positions.value(ks.at(i));
		long position = 0;
		if(size(pair) == 1){
			position = pair.first;
		} else if(size(pair) == 2){
			position = pair.first + pair.second;
			position /= 2;
		}

		result[yaw(ks.at(i))].push_back(position);
	}

	long position = 0;
	long eyes = 0;
	if(size(positionsFromFile(filename)) == 1){
		position = positionsFromFile(filename).first;
		eyes = 1;
	} else if(size(positionsFromFile(filename)) == 2){
		position = positionsFromFile(filename).first + positionsFromFile(filename).second;
		position /= 2;
		eyes = 2;
	}

	auto keys = result.keys();
	QVector<long> support(keys.count(), 0);
	for(int i=0; i<keys.count(); ++i){
		auto values = result.value(keys.at(i));
		for(int j=0; j<values.count(); ++j){
			double a = std::abs(values.at(j) - position);
			if(a < fuzziness){
				support.replace(i, support.at(i)+1);
			}
		}
	}

	return support;
}

bool YawDetector::containsTies(QVector<long> vec){
	int max = 0;

	foreach(long v, vec){
		if(v>max){
			max = v;
		}
	}

	return (vec.count(max) > 1) ? true : false;
}

long YawDetector::newDetect(QString filename, double fuzziness){
	auto support = detectYaw(filename, fuzziness);
	int counter = 0;
	//	while(containsTies(support) && fuzziness > 0){
	//		++counter;
	//		fuzziness -= 0.0002;
	//		support = detectYaw(filename, fuzziness);
	//	}
	//	while(containsTies(support)){
	//		++counter;
	//		fuzziness += 0.002;
	//		support = detectYaw(filename, fuzziness);
	//	}

	QVector<long> keys;
	keys << -90 << -45 << 0 << 10 << 20 << 30 << 45 << 90;
	for(int i=0; i<keys.count(); ++i){
		std::cout << keys.at(i) << " -> " << support.at(i) << std::endl;
	}

	//TODO return the yaw
}

long YawDetector::operator()(QString image){
	std::cout << "First " << positionsFromFile(image).first << " Second " << positionsFromFile(image).second << std::endl;
	long eyes1Size = size(positionsFromFile(image));
	long position1 = positionsFromFile(image).first + positionsFromFile(image).second;

	QMultiMap<long, QString> scores;
	auto images = positions.keys();
	foreach(QString image, images){
		auto pair = positions.value(image);
		long score = 0;

		if(eyes1Size != size(pair) || eyes1Size == 0 || size(pair) == 0){
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
		std::cout << "This should never happen" << std::endl;
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
//		vector<Rect> eyes = Image::detectEyes(image);
//		if(eyes.size() == 0){
//			result.insert(imagePath, qMakePair(0, 0));
//		} else if(eyes.size() == 1){
//			result.insert(imagePath, qMakePair(Image::getRelativePositionEye(image, eyes.at(0)), 0));
//		} else {
//			result.insert(imagePath, qMakePair(Image::getRelativePositionEye(image, eyes.at(0)), Image::getRelativePositionEye(image, eyes.at(1))));
//		}
		QString landmarksPath(imagePath);
		landmarksPath.replace(".png", ".lm2");
		std::cout << "Detect " << imagePath.toStdString() << std::endl;
		LandMarkReader lmr = LandMarkReader(landmarksPath);
		long leftOuter = lmr.leftEyeCorner().first;
		long leftInner = lmr.innerLeftEyeCorner().first;
		long rightOuter = lmr.rightEyeCorner().first;
		long rightInner = lmr.innerRightEyeCorner().first;

		long left = (leftOuter + leftInner)/2/double(image.cols)*100;
		long right = (rightOuter + rightInner)/2/double(image.cols)*100;

		result.insert(imagePath, qMakePair(left, right));
	}
}

return result;
}

void YawDetector::serialize(QMap<QString, QPair<long, long> > positions){
	QFile file("positionsYawOrig");
	if(file.open(QIODevice::WriteOnly)){
		QTextStream stream(&file);

		for(int i=0; i<positions.count(); ++i){
			auto key = positions.keys().at(i);
			auto value = positions.value(key);

			if(value.first > value.second){
				stream << key << "\n" << value.first << "\n" << value.second << "\n";
			} else {
				stream << key << "\n" << value.second << "\n" << value.first << "\n";
			}
		}
	}

	file.close();
}

QMap<QString, QPair<long, long> > YawDetector::deserialize(){
	QMap<QString, QPair<long, long> > result;

	QFile file("positionsYawOrig");
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

QPair<long, long> YawDetector::positionsFromFile(QString filename){
	QFileInfo info1(filename);
	auto allPositions = deserialize();
	auto keys = allPositions.keys();
	for(int i=0; i<keys.count(); ++i){
		QFileInfo info2(keys.at(i));
		if(info1.fileName() == info2.fileName()){
			return allPositions[keys.at(i)];
		}
	}
}
