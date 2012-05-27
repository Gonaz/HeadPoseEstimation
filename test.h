#ifndef TEST_H
#define TEST_H


void test(){
	QString filename = "../HeadPoseEstimation/data/bs011/bs011_PR_D_0.png";
	Mat image = imread(filename.toStdString());
	vector<Rect> eyes = Image::detectEyes(image);
	vector<Rect> nose = Image::detectNose(image);
	vector<Rect> mouth = Image::detectMouth(image);

	std::cout << "Eyes " << eyes.size() << std::endl;
	for(int i=0; i<eyes.size(); ++i){
		rectangle(image, eyes.at(i), Scalar(200, 0, 0), 3);
	}

	std::cout << "Nose " << nose.size() << std::endl;
	for(int i=0; i<nose.size(); ++i){
		rectangle(image, nose.at(i), Scalar(0, 200, 0), 3);
	}

	std::cout << "Mouth " << mouth.size() << std::endl;
	for(int i=0; i<mouth.size(); ++i){
		rectangle(image, mouth.at(i), Scalar(0,0,200), 3);
	}

	//	PitchTrainer pt = false;
	//	vector<double> fts = pt.detectFeatures(filename);
	//	for(int i=0; i<fts.size(); i+=2){
	//		circle(image, Point(fts[i], fts[i+1]), 5, Scalar(0, 200, 200), 3);
	//	}

	vector<double> features(10);
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

	//	std::cout << distanceMouthNose(features, image)/distanceNoseEye(features, image) << std::endl;

	resize(image, image, Size(image.cols*0.6, image.rows*0.6));
	imshow("Image", image);
	waitKey();
}

void test2(){
	QString filename = "/home/jonas/Qt/HeadPoseEstimation/data/bs017/bs017_PR_U_0.png";
	Mat img, gray;
	img = imread(filename.toStdString());
	cvtColor(img, gray, CV_BGR2GRAY);
	threshold(gray, gray, 40, 255, THRESH_BINARY);
	//	Canny(gray, gray, 0, 60);

	// smooth it, otherwise a lot of false circles may be detected
	//	GaussianBlur( gray, gray, Size(9, 9), 2);
	vector<Vec3f> circles;
	HoughCircles(gray, circles, CV_HOUGH_GRADIENT, 2, gray.rows/4);
	std::cout << "Circles " << circles.size() << std::endl;
	for(size_t i = 0; i<circles.size(); i++){
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		circle( img, center, 3, Scalar(0,255,0), -1, 8, 0 );
		circle( img, center, radius, Scalar(0,0,255), 3, 8, 0 );
	}

	resize(img, img, Size(img.cols*0.6, img.rows*0.6));
	imshow("circles", img);
	waitKey();
}

//---------------------------------------------------------------------------------------------
std::ostream& operator<<(std::ostream& str, const QString& string) {
	return str << string.toStdString();
}

QMap<long, QVector<double> > values;

double overlap(){
	auto keys = values.keys();
	double result = 0;

	for(int i=0; i<keys.size(); ++i){
		for(int j=i+1; j<keys.size(); ++j){
			QVector<double> first = values.value(keys[i]);
			QVector<double> second = values.value(keys[j]);

			auto mmFirst = std::minmax_element(first.begin(), first.end());
			auto mmSecond = std::minmax_element(second.begin(), second.end());

			double innerMax = (*(mmFirst.second) < *(mmSecond.second)) ? *(mmFirst.second) : *(mmSecond.second);
			double size1 = *(mmFirst.second) - *(mmFirst.first) + 1;
			double size2 = *(mmSecond.second) - *(mmSecond.first) +1;
			double tmp = 0;
			if(*(mmFirst.first) <= *(mmSecond.first)){
				tmp = innerMax - *(mmSecond.first) + 1;
			} else {
				tmp = innerMax - *(mmFirst.first) + 1;
			}
			tmp /= std::min(size1, size2);
			result += tmp;
		}
	}

	return result;
}

QMap<QString, QMap<QString, double> > information; //filename -> parameter -> value
QStringList landmarks = {"Outer left eyebrow", "Middle left eyebrow", "Inner left eyebrow", "Inner right eyebrow", "Middle right eyebrow", "Outer right eyebrow", "Outer left eye corner", "Inner left eye corner", "Inner right eye corner", "Outer right eye corner", "Nose saddle left", "Nose saddle right", "Left nose peak", "Nose tip", "Right nose peak", "Left mouth corner", "Upper lip outer middle", "Right mouth corner", "Upper lip inner middle", "Lower lip inner middle", "Lower lip outer middle", "Chin middle"};

void readInformation(){
	QString base = "../HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";

	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		foreach(QString image, images){
			if(!image.contains("YR")){
				QString imageFile = imageDir + "/" + image;
				QString landMarkFile = imageFile;
				landMarkFile.replace(".png", ".lm2");
				LandMarkReader lmr = landMarkFile;
				Mat image = imread(imageFile.toStdString());
				for(QString landmark : landmarks){
					information[imageFile][landmark] = lmr.fetchValue(landmark).second/double(image.rows)*100;
				}
			}
		}
	}
}

double operation(std::vector<double> v){
	//	return v[0] - v[1];
	if((v[1] - v[2]) == 0){
		return 0;
	}
	return (v[0] - v[1]) / (v[1] - v[2]);
}

void test3(){
	QMap<std::vector<int>, double> scores;
	double best = 100;
	readInformation();

	for(int i=0; i<landmarks.size(); ++i){
		for(int j=0; j<landmarks.size(); ++j){
			for(int k=0; k<landmarks.size(); ++k){
				double max = -INFINITY;
				values.clear();
				QString landmark1 = landmarks[i];
				QString landmark2 = landmarks[j];
				QString landmark3 = landmarks[k];
				auto keys = information.keys();
				for(QString key : keys){
					long pitch = PitchDetector::pitch(key);
					double v = operation({information[key][landmark1], information[key][landmark2], information[key][landmark3]});
				values[pitch].append(v);
				if(v > max){
					max = v;
				}
			}
			for(int i=0; i<values.keys().size(); ++i){
				auto key = values.keys().at(i);
				for(auto &value : values[key]){
					value /= max;
				}
			}
			scores[std::vector<int>{i,j,k}] = overlap();
			if(overlap() < best){
				best = overlap();
			}
		}
	}
}
std::cout << best << " -> " << std::endl;
auto keys = scores.keys(best);
for(auto key : keys){
	std::cout << "\t";
	for(auto landmark : key){
		std::cout << landmark << " ";
	}
	for(auto landmark : key){
		std::cout << landmarks[landmark] << " & ";
	}
	std::cout << std::endl;
}

}

void test4(){
	readInformation();
	int i = landmarks.indexOf("Outer left eyebrow");
	int j = landmarks.indexOf("Middle left eyebrow");
	int k = landmarks.indexOf("Nose tip");
	QString landmark1 = landmarks[i];
	QString landmark2 = landmarks[j];
	QString landmark3 = landmarks[k];
	auto keys = information.keys();
	double max = -100;
	for(QString key : keys){
		long pitch = PitchDetector::pitch(key);
		double v = operation({information[key][landmark1], information[key][landmark2], information[key][landmark3]});
	values[pitch].append(v);
	if(v > max){
		max = v;
	}
}
for(int i=0; i<values.keys().size(); ++i){
	auto key = values.keys().at(i);
	for(auto &value : values[key]){
		value /= max;
	}
}

std::cout << overlap() << std::endl;
}

double distance(QPair<double, double> p1, QPair<double, double> p2){
	return sqrt(pow(p1.first - p2.first, 2) + pow(p1.second - p2.second, 2));
}

QPair<long, long> intersection(QPair<long, long> p1, QPair<long, long> p2, QPair<long, long> p3, QPair<long, long> p4){
	float xD1,yD1,xD2,yD2,xD3,yD3;
	float dot,deg,len1,len2;
	float segmentLen1,segmentLen2;
	float ua,ub,div;

	// calculate differences
	xD1=p2.first-p1.first;
	xD2=p4.first-p3.first;
	yD1=p2.second-p1.second;
	yD2=p4.second-p3.second;
	xD3=p1.first-p3.first;
	yD3=p1.second-p3.second;

	// calculate the lengths of the two lines
	len1=sqrt(xD1*xD1+yD1*yD1);
	len2=sqrt(xD2*xD2+yD2*yD2);

	// calculate angle between the two lines.
	dot=(xD1*xD2+yD1*yD2); // dot product
	deg=dot/(len1*len2);

	// if abs(angle)==1 then the lines are parallell,
	// so no intersection is possible
	if(abs(deg)==1){
		std::cout << "Error 1" << std::endl;
	}

	// find intersection Pt between two lines
	QPair<long, long> pt = qMakePair(0l,0l);
	div=yD2*xD1-xD2*yD1;
	ua=(xD2*yD3-yD2*xD3)/div;
	ub=(xD1*yD3-yD1*xD3)/div;
	pt.first=p1.first+ua*xD1;
	pt.second=p1.second+ua*yD1;

	// calculate the combined length of the two segments
	// between Pt-p1 and Pt-p2
	xD1=pt.first-p1.first;
	xD2=pt.first-p2.first;
	yD1=pt.second-p1.second;
	yD2=pt.second-p2.second;
	segmentLen1=sqrt(xD1*xD1+yD1*yD1)+sqrt(xD2*xD2+yD2*yD2);

	// calculate the combined length of the two segments
	// between Pt-p3 and Pt-p4
	xD1=pt.first-p3.first;
	xD2=pt.first-p4.first;
	yD1=pt.second-p3.second;
	yD2=pt.second-p4.second;
	segmentLen2=sqrt(xD1*xD1+yD1*yD1)+sqrt(xD2*xD2+yD2*yD2);

	// if the lengths of both sets of segments are the same as
	// the lenghts of the two lines the point is actually
	// on the line segment.

	// if the point isn't on the line, return null
	if(abs(len1-segmentLen1)>0.01 || abs(len2-segmentLen2)>0.01){
//		std::cout << "Error 2 " << std::endl;
	}

	// return the valid intersection
	return pt;
}

void test5(){
	QString base = "../HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";
	QVector<double> lns;
	QVector<double> lfs;
	QVector<double> lms;

	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		auto image = images.first();
		{
			QString filename = imageDir + "/" + image;
			auto split = filename.split("_");
			auto newFilename = split.first() + "_YR_L90_0.png";
			QString landmarks = newFilename;
			landmarks.replace(".png", ".lm2");
			LandMarkReader lmr = landmarks;
			Mat image = imread(newFilename.toStdString());

			auto point1 = lmr.fetchValue("Left nose peak");
			auto p1 = qMakePair(point1.first/double(image.cols)*100, point1.second/double(image.rows)*100);

			auto point2 = lmr.fetchValue("Nose tip");
			auto p2 = qMakePair(point2.first/double(image.cols)*100, point2.second/double(image.rows)*100);

			auto point3 = lmr.fetchValue("Outer left eye corner");
			auto p3 = qMakePair(point3.first/double(image.cols)*100, point3.second/double(image.rows)*100);

			auto point4 = lmr.fetchValue("Left mouth corner");
			auto p4 = qMakePair(point4.first/double(image.cols)*100, point4.second/double(image.rows)*100);

			auto point5 = intersection(point3, point4, point1, point2);
			auto p5 = qMakePair(point5.first/double(image.cols)*100, point5.second/double(image.rows)*100);

			if(not((point1.first == 0 and point1.second == 0) or (point2.first == 0 and point2.second == 0))){
				lns.append(distance(p1, p2));
				lfs.append(distance(p3, p4));
				lms.append(distance(p5, p4));
			}
		}
	}

	std::cout << lns.size() << std::endl;

	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		auto image = images.first();
		{
			QString filename = imageDir + "/" + image;
			auto split = filename.split("_");
			auto newFilename = split.first() + "_YR_R90_0.png";
			QString landmarks = newFilename;
			landmarks.replace(".png", ".lm2");
			LandMarkReader lmr = landmarks;
			Mat image = imread(newFilename.toStdString());

			auto point1 = lmr.fetchValue("Right nose peak");
			auto p1 = qMakePair(point1.first/double(image.cols)*100, point1.second/double(image.rows)*100);

			auto point2 = lmr.fetchValue("Nose tip");
			auto p2 = qMakePair(point2.first/double(image.cols)*100, point2.second/double(image.rows)*100);

			auto point3 = lmr.fetchValue("Outer right eye corner");
			auto p3 = qMakePair(point3.first/double(image.cols)*100, point3.second/double(image.rows)*100);

			auto point4 = lmr.fetchValue("Right mouth corner");
			auto p4 = qMakePair(point4.first/double(image.cols)*100, point4.second/double(image.rows)*100);

			auto point5 = intersection(point3, point4, point1, point2);
			auto p5 = qMakePair(point5.first/double(image.cols)*100, point5.second/double(image.rows)*100);

			if(not((point1.first == 0 and point1.second == 0) or (point2.first == 0 and point2.second == 0))){
				lns.append(distance(p1, p2));
				lfs.append(distance(p3, p4));
				lms.append(distance(p5, p4));
			}
		}
	}

	std::cout << lns.size() << std::endl;

	double sum = 0;
	for(int i=0; i<lns.size(); ++i){
		sum += (lns.at(i) / lfs.at(i));
	}
	std::cout << sum/lns.size() << std::endl;

	sum = 0;
	for(int i=0; i<lms.size(); ++i){
		sum += (lms.at(i) / lfs.at(i));
	}
	std::cout << sum/lms.size() << std::endl;
}


#endif // TEST_H
