#ifndef IMAGE_H
#define IMAGE_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>

namespace image{
	using namespace cv;

	vector<Rect> detectEyes(Mat image);
	long getRelativePositionEye(Mat image, Rect eye);
	vector<Rect> detect(Mat image, QString file, size_t amount=1);

	long getRelativePositionEye(Mat image, Rect eye){
		long center = eye.x + (eye.width/2);
		return center/double(image.cols)*100;
	}

	vector<Rect> detectEyes(Mat image){
		//TODO adaptive resize
		double scale = 0.4;
		double iScale = 1/scale;
		resize(image, image, Size(image.cols*scale, image.rows*scale));
		vector<Rect> eyes = detect(image, "/usr/local/share/OpenCV/haarcascades/haarcascade_eye.xml", 2);
		if(eyes.size() > 1){
			if(eyes.at(0).tl().y > eyes.at(1).br().y){
				eyes.erase(eyes.begin());
			} else if(eyes.at(1).tl().y > eyes.at(0).br().y ){
				eyes.erase(eyes.end());
			}
		}

		for(size_t i=0; i<eyes.size(); ++i){
			Rect r = eyes.at(i);
			Rect newR(r.x*iScale, r.y*iScale, r.width*iScale, r.height*iScale);
			eyes.at(i) = newR;
		}

		return eyes;
	}

	vector<Rect> detect(Mat image, QString file, size_t amount){
		vector<Rect> rectangles;
		CascadeClassifier cc(file.toStdString());
		cc.detectMultiScale(image, rectangles);
		int param = 5;
		while(rectangles.size() > amount){
			cc.detectMultiScale(image, rectangles, 1.1, param);
			param += 5;
		}

		return rectangles;
	}

}//namespace image
#endif // IMAGE_H
