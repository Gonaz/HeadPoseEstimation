#include <QtGui/QApplication>
#include "mainwindow.h"
#include <QDir>
#include <QTextStream>
#include <map>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream>

#define ONE_MILLION 1000000

using namespace cv;

QString base = "/home/jonas/Qt/HeadPoseEstimation/data";

QStringList subdirectories(){
	QDir dir(base);
	QStringList subdirs = dir.entryList();
	for(int i=0; i<subdirs.count(); ++i){
		if(!subdirs.at(i).startsWith("bs00")){
			subdirs.removeAt(i--);
		}
	}

	return subdirs;
}

QMap<QString, Mat> imagesFromDirectory(QString directory){
	QMap<QString, Mat> imgs;
	QDir imageDir(directory);
	QStringList images = imageDir.entryList(QStringList("*.png"));
	foreach(QString image, images){
		QString imagePath = directory + "/" + image;
		Mat img = imread(imagePath.toStdString());
		imgs[imagePath] = img;
	}
	return imgs;
}

void serialize(QMap<QString, QPair<long, long> > positions){
	QFile file("positions");
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

QMap<QString, QPair<long, long> > deserialize(){
	QMap<QString, QPair<long, long> > result;

	QFile file("positions");
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

long compare(Mat image1, Mat image2){
	assert(image1.type() == CV_8UC1 && image2.type() == CV_8UC1);
	int rows = (image1.rows < image2.rows) ? image1.rows : image2.rows;
	int cols = (image1.cols < image2.cols) ? image1.cols : image2.cols;

	long result = 0;
	for(int i=0; i<rows; ++i){
		for(int j=0; j<cols; ++j){
			int val1 = int(image1.at<uchar>(Point(j,i)));
			int val2 = int(image2.at<uchar>(Point(j,i)));
			result += (val1-val2);
		}
	}

	return result;
}

Mat canny(Mat input){
	Mat tmp, result;
	GaussianBlur(input, tmp, Size(5,5), 2);
	cvtColor(tmp, tmp, CV_BGR2GRAY);
	Canny(tmp, result, 20, 35);
	return result;
}

Mat canny(Mat input, int scale){
	Mat result = canny(input);
	resize(result, result, Size(input.cols*double(scale)/100, input.rows*double(scale)/100));
	return result;
}

QMultiMap<long, QString> compare(Mat image1){
	QString base = "/home/jonas/Qt/HeadPoseEstimation/data";

	QDir dir(base);
	QStringList subdirs = dir.entryList();
	for(int i=0; i<subdirs.count(); ++i){
		if(!subdirs.at(i).startsWith("bs000")){
			subdirs.removeAt(i--);
		}
	}

	QMultiMap<long, QString> scores;
	foreach(QString subdir, subdirs){
		std::cout << "subdir " << subdir.toStdString() << std::endl;
		QDir imageDir(base + "/" + subdir);
		QStringList images = imageDir.entryList(QStringList("*.png"));
		foreach(QString image, images){
			QString imagePath = base + "/" + subdir + "/" + image;
			Mat image2 = imread(imagePath.toStdString());
			Mat tmp1 = canny(image1);
			Mat tmp2 = canny(image2);
			long score = compare(tmp1, tmp2);
			scores.insertMulti(abs(score), imagePath);
		}
	}

	return scores;
}

template<class T>
QPair<T, QString> getBest(QMultiMap<T, QString> scores, int number=0){
	QList<T> keys = scores.keys();
	qSort(keys.begin(), keys.end());
	return qMakePair(keys.at(number), scores.value(keys.at(number)));
}

template<class T>
std::ostream& operator<< (std::ostream &out, QPair<T, QString> pair) {
	out << "(" << pair.first << "; " << pair.second.toStdString() << ")";
	return out;
}

Mat merge(Mat image1, Mat image2){
	int rows = (image1.rows > image2.rows) ? image1.rows : image2.rows;
	int cols = (image1.cols > image2.cols) ? image1.cols : image2.cols;
	Mat result(rows, cols, CV_8UC3);

	for(int i=0; i<rows; ++i){
		for(int j=0; j<cols; ++j){
			result.at<Vec3b>(Point(j, i))[0] = 0;
			result.at<Vec3b>(Point(j, i))[1] = 0;
			result.at<Vec3b>(Point(j, i))[2] = 0;
		}
	}

	for(int i=0; i<image1.rows; ++i){
		for(int j=0; j<image1.cols; ++j){
			result.at<Vec3b>(Point(j, i))[0] = image1.at<uchar>(Point(j, i));
		}
	}

	for(int i=0; i<image2.rows; ++i){
		for(int j=0; j<image2.cols; ++j){
			result.at<Vec3b>(Point(j, i))[1] = image2.at<uchar>(Point(j, i));
		}
	}

	return result;
}

Mat merge(Mat image1, Mat image2, int scale){
	Mat result = merge(image1, image2);
	resize(result, result, Size(result.cols*double(scale)/100, result.rows*double(scale)/100));
	return result;
}

void calculateOffset(Mat image1, Mat image2, int& rows, int& cols){

}

vector<Rect> detect(Mat image, String file, int amount=1){
	vector<Rect> rectangles;
	CascadeClassifier cc(file);
	cc.detectMultiScale(image, rectangles);
	int param = 5;
	while(rectangles.size() > amount){
		cc.detectMultiScale(image, rectangles, 1.1, param);
		param += 5;
	}

	return rectangles;
}

double whiteness(Mat image){
	Mat result;
	cvtColor(image, result, CV_BGR2GRAY);
	threshold(result, result, 127, 255, THRESH_OTSU);

	double whiteness = 0.0;
	for(int i=0; i<result.rows; ++i){
		for(int j=0; j<result.cols; ++j){
			if(result.at<uchar>(Point(j,i)) == 255){
				++whiteness;
			}
		}
	}

	whiteness /= (result.cols * result.rows);
	whiteness *= ONE_MILLION;
	return whiteness;
}

vector<Rect> detectEyes(Mat image){
	//TODO adaptive resize
	double scale = 0.4;
	double iScale = 1/scale;
	resize(image, image, Size(image.cols*scale, image.rows*scale));
	vector<Rect> eyes = detect(image, "/usr/local/share/OpenCV/haarcascades/haarcascade_eye.xml", 2);
	if(eyes.size() > 1){
		if(eyes.at(0).tl().y > eyes.at(1).br().y){
//			std::cout << "Remove first eye" << std::endl;
			eyes.erase(eyes.begin());
		} else if(eyes.at(1).tl().y > eyes.at(0).br().y ){
//			std::cout << "Remove second eye" << std::endl;
			eyes.erase(eyes.end());
		}
	}

	for(int i=0; i<eyes.size(); ++i){
		Rect r = eyes.at(i);
		Rect newR(r.x*iScale, r.y*iScale, r.width*iScale, r.height*iScale);
		eyes.at(i) = newR;
	}

//	std::cout << "Return number of eyes " << eyes.size() << std::endl;
	return eyes;
}

long getRelativePositionEye(Mat image, Rect eye){
	long center = eye.x + (eye.width/2);
	return center/double(image.cols)*100;
}

QMultiMap<long, QString> detectYaw(Mat image1){
	QStringList subdirs = subdirectories();

	QMultiMap<long, QString> scores;
	foreach(QString subdir, subdirs){
		std::cout << "subdir " << subdir.toStdString() << std::endl;
		QDir imageDir(base + "/" + subdir);
		QStringList images = imageDir.entryList(QStringList("*.png"));
		foreach(QString image, images){
			QString imagePath = base + "/" + subdir + "/" + image;
			Mat image2 = imread(imagePath.toStdString());
			long score = whiteness(image1) - whiteness(image2);
			scores.insertMulti(abs(score), imagePath);
		}
	}

	return scores;
}

QMultiMap<long, QString> detectYaw2(Mat image1){
	QStringList subdirs = subdirectories();

	QMultiMap<long, QString> scores;
	foreach(QString subdir, subdirs){
		std::cout << "subdir " << subdir.toStdString() << std::endl;
		QDir imageDir(base + "/" + subdir);
		QStringList images = imageDir.entryList(QStringList("*.png"));
		foreach(QString image, images){
			QString imagePath = base + "/" + subdir + "/" + image;
			Mat image2 = imread(imagePath.toStdString());

			vector<Rect> eyes1 = detectEyes(image1);
			vector<Rect> eyes2 = detectEyes(image2);

			long score = 0;

			if(eyes1.size() != eyes2.size() || eyes1.size() == 0 || eyes2.size() == 0){
				score = ONE_MILLION;
			} else {
				long position1 = 0;
				for(int i=0; i<eyes1.size(); ++i){
					position1 += getRelativePositionEye(image1, eyes1.at(i));
				}
				position1 /= eyes1.size();

				long position2 = 0;
				for(int i=0; i<eyes2.size(); ++i){
					position2 += getRelativePositionEye(image2, eyes2.at(i));
				}
				position2 /= eyes2.size();

				score = (position1 - position2);
			}

			scores.insertMulti(abs(score), imagePath);
		}
	}

	return scores;
}

int size(QPair<long, long> pair){
	if(pair.first == -1){
		return 0;
	} else if(pair.second == -1){
		return 1;
	}
	return 2;
}

QMultiMap<long, QString> detectYaw2(Mat image1, QMap<QString, QPair<long, long> > positions){
	QStringList subdirs = subdirectories();

	vector<Rect> eyes1 = detectEyes(image1);
	long position1 = 0;
	for(int i=0; i<eyes1.size(); ++i){
		position1 += getRelativePositionEye(image1, eyes1.at(i));
	}
	if(eyes1.size() > 0){
		position1 /= eyes1.size();
	}

	QMultiMap<long, QString> scores;
	foreach(QString subdir, subdirs){
		QDir imageDir(base + "/" + subdir);
		QStringList images = imageDir.entryList(QStringList("*.png"));
		foreach(QString image, images){
			QString imagePath = base + "/" + subdir + "/" + image;
			auto pair = positions.value(imagePath);
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

			scores.insertMulti(abs(score), imagePath);
		}
	}

	return scores;
}

int yaw(QString filename){
	int rot;
	filename = filename.split("/").last();
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

QMap<QString, QPair<long, long> > calculateRelativePositions(){
	QMap<QString, QPair<long, long> > result;
	QStringList subdirs = subdirectories();

	foreach(QString subdir, subdirs){
		std::cout << "subdir " << subdir.toStdString() << std::endl;
		QDir imageDir(base + "/" + subdir);
		QStringList images = imageDir.entryList(QStringList("*.png"));
		foreach(QString image, images){
			QString imagePath = base + "/" + subdir + "/" + image;
			Mat image = imread(imagePath.toStdString());
			vector<Rect> eyes = detectEyes(image);
			if(eyes.size() == 0){
				result.insert(imagePath, qMakePair(-1l, -1l));
			} else if(eyes.size() == 1){
				result.insert(imagePath, qMakePair(getRelativePositionEye(image, eyes.at(0)), -1l));
			} else {
				result.insert(imagePath, qMakePair(getRelativePositionEye(image, eyes.at(0)), getRelativePositionEye(image, eyes.at(1))));
			}
		}
	}

	return result;
}

void crossValidateYaw(){
//	auto positions = calculateRelativePositions();
//	serialize(positions);
	auto positions = deserialize();

	std::cout << "Finished preprocessing" << std::endl;
	int correct = 0;
	int wrong = 0;

	QString extendedBase = base + "/" + "bs01";
	for(int i=0; i<10; ++i){
		QString imageDir = extendedBase + QString::number(i);
		QMap<QString,Mat> images = imagesFromDirectory(imageDir);
		for(int j=0; j<images.count(); ++j){
			auto key = images.keys().at(j);
			std::cout << key.toStdString() << std::endl;
			auto best = getBest(detectYaw2(images.value(key), positions));
			int y = yaw(best.second);
			std::cout << "Best " << best << std::endl;
			std::cout << "Estimation " << y << "\t Real " << yaw(key) << std::endl;
			if(y != yaw(key)){
				++wrong;
			} else {
				++correct;
			}
		}
	}

	std::cout << correct << "/" << (correct+wrong) << std::endl;
}

int main(int argc, char *argv[]) {
	crossValidateYaw();

	//	QApplication a(argc, argv);
	//	MainWindow w;
	//	w.showMaximized();
	//	return a.exec();

	//	Mat img0 = imread("../HeadPoseEstimation/data/bs000/bs000_N_N_0.png");
	//	resize(img0, img0, Size(img0.cols*double(40)/100, img0.rows*double(40)/100));

	//	Mat img0 = imread("/home/jonas/Qt/HeadPoseEstimation/data/lena.jpg");
//	Mat img0 = imread("/home/jonas/Qt/HeadPoseEstimation/data/bs003/bs003_YR_L45_0.png");

	//		QMultiMap<long, QString> scores = compare(img0);
	//		for(int i=0; i<scores.keys().count(); ++i){
	//			std::cout << getBest(scores, i) << std::endl;
	//		}

	//		Mat img1 = imread(getBest(scores, 2).second.toStdString());
	////			namedWindow("original");
	//		//	imshow("original", canny(img0, 40));
	//		//	namedWindow("best");
	//		//	imshow("best", canny(img1, 40));

	//		namedWindow("merged");
	//		imshow("merged", merge(canny(img0), canny(img1), 60));



//	auto scores = detectYaw2(img0);
//	std::cout << getBest(scores,1) << std::endl;
//	std::cout << getBest(scores,2) << std::endl;
//	Mat img1 = imread(getBest(scores, 1).second.toStdString());

	//	resize(img0, img0, Size(img0.cols*double(40)/100, img0.rows*double(40)/100));
	//	vector<Rect> eyes = detect(img0, "/usr/local/share/OpenCV/haarcascades/haarcascade_eye.xml", 2);


	//	resize(img1, img1, Size(img1.cols*double(40)/100, img1.rows*double(40)/100));
	//	eyes = detect(img1, "/usr/local/share/OpenCV/haarcascades/haarcascade_eye.xml", 2);
	//	for(size_t i=0; i<eyes.size(); ++i){
	//		rectangle(img1, eyes.at(i), Scalar(255,0,0));
	//	}

//	imshow("original", img0);
//	imshow("best", img1);









	//		vector<Rect> faces = detect(img0, "/usr/local/share/OpenCV/haarcascades/haarcascade_frontalface_alt.xml");
	//		vector<Rect> eyes = detect(img0, "/usr/local/share/OpenCV/haarcascades/haarcascade_eye.xml", 2);
	//		vector<Rect> mouths = detect(img0, "/usr/local/share/OpenCV/haarcascades/haarcascade_mcs_mouth.xml");
	//		vector<Rect> noses = detect(img0, "/usr/local/share/OpenCV/haarcascades/haarcascade_mcs_nose.xml");

	//		std::cout << faces.size() << "|" << eyes.size() << "|" << mouths.size() << "|" << noses.size() << std::endl;

	//		for(size_t i=0; i<faces.size(); ++i){
	//			rectangle(img0, faces.at(i), Scalar(0,255,0));
	//		}
	//		for(size_t i=0; i<eyes.size(); ++i){
	//			rectangle(img0, eyes.at(i), Scalar(255,0,0));
	//		}
	//		for(size_t i=0; i<mouths.size(); ++i){
	//			rectangle(img0, mouths.at(i), Scalar(0,0,255));
	//		}
	//		for(size_t i=0; i<noses.size(); ++i){
	//			rectangle(img0, noses.at(i), Scalar(0,255,255));
	//		}


	//		namedWindow("haar");
	//		imshow("haar", img0);

//	waitKey(0);
	return 0;
}
