#ifndef IMAGE_H
#define IMAGE_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <iostream> //TODO debug

class Image{
public:

	static long getRelativePositionEye(cv::Mat image, cv::Rect eye){
		long center = eye.x + (eye.width/2);
		return center/double(image.cols)*100;
	}

	static cv::vector<cv::Rect> detectEyes(cv::Mat image){
		//TODO adaptive resize
		double scale = 0.4;
		double iScale = 1/scale;
		cv::resize(image, image, cv::Size(image.cols*scale, image.rows*scale));
//		cv::GaussianBlur(image, image, cv::Size(7,7), 0);
		cv::vector<cv::Rect> eyes = detect(image, "/usr/local/share/OpenCV/haarcascades/haarcascade_eye.xml", 2);
		if(eyes.size() > 1){
			if(eyes.at(0).tl().y > eyes.at(1).br().y){
				eyes.erase(eyes.begin());
			} else if(eyes.at(1).tl().y > eyes.at(0).br().y ){
				eyes.erase(eyes.end());
			}
		}

		for(size_t i=0; i<eyes.size(); ++i){
			cv::Rect r = eyes.at(i);
			cv::Rect newR(r.x*iScale, r.y*iScale, r.width*iScale, r.height*iScale);
			eyes.at(i) = newR;
		}

		return eyes;
	}

	static cv::vector<cv::Rect> detectMouth(cv::Mat image){
		//TODO adaptive resize
		double scale = 0.4;
		double iScale = 1/scale;
		cv::resize(image, image, cv::Size(image.cols*scale, image.rows*scale));
		cv::vector<cv::Rect> mouth = detect(image, "/usr/local/share/OpenCV/haarcascades/haarcascade_mcs_mouth.xml", 1);

		for(auto it=mouth.begin(); it != mouth.end(); ++it){
			if((*it).tl().y < image.rows/2){
				mouth.erase(it--);
			}
		}

		for(size_t i=0; i<mouth.size(); ++i){
			cv::Rect r = mouth.at(i);
			cv::Rect newR(r.x*iScale, r.y*iScale, r.width*iScale, r.height*iScale);
			mouth.at(i) = newR;
		}

		return mouth;
	}

	static cv::vector<cv::Rect> detect(cv::Mat image, QString file, size_t amount){
		cv::vector<cv::Rect> rectangles;
		cv::CascadeClassifier cc(file.toStdString());
		cc.detectMultiScale(image, rectangles);
		int param = 5;
		while(rectangles.size() > amount){
			cc.detectMultiScale(image, rectangles, 1.1, param);
			param += 5;
		}

		return rectangles;
	}

};

#endif // IMAGE_H
