#ifndef IMAGE_H
#define IMAGE_H

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp> //TODO: debug
#include <iostream> //TODO: debug
#include <QString>

class Image{
public:

	static long getRelativePositionEye(cv::Mat image, cv::Rect eye){
		long center = eye.x + (eye.width/2);
		return center/double(image.cols)*100;
	}

	static cv::Rect getLeftEye(cv::vector<cv::Rect> v){ //TODO: duplicate code with getRightEye
		if(v.size() == 0){
			throw std::exception();
			return cv::Rect(-1, -1, -1, -1);
		} else if(v.size() == 1){
			throw std::exception();
			return v.at(0);
		} else {
			assert(v.size() == 2); //TODO: debug
			return (v.at(0).x < v.at(1).x) ? v.at(0) : v.at(1);
		}
	}

	static cv::Rect getRightEye(cv::vector<cv::Rect> v){ //TODO: duplicate code with getLeftEye
		if(v.size() == 0){
			throw std::exception();
			return cv::Rect(-1, -1, -1, -1);
		} else if(v.size() == 1){
			throw std::exception();
			return v.at(0);
		} else {
			assert(v.size() == 2); //TODO: debug
			return (v.at(0).x > v.at(1).x) ? v.at(0) : v.at(1);
		}
	}


	static cv::vector<cv::Rect> detectEyes(cv::Mat image){
		cv::vector<cv::Rect> eyes;
		double scale = 0.2;
		while(eyes.size() == 0 && scale <= 1){
			double iScale = 1/scale;
			cv::Mat image2;
			cv::resize(image, image2, cv::Size(image.cols*scale, image.rows*scale));
			eyes = detect(image2, "/usr/local/share/OpenCV/haarcascades/haarcascade_eye.xml", 2);
			if(eyes.size() > 1){
				if(eyes.at(0).tl().y > eyes.at(1).br().y){
					eyes.erase(eyes.begin());
				} else if(eyes.at(1).tl().y > eyes.at(0).br().y ){
					eyes.erase(eyes.end());
				}
				if(eyes.size() > 1){ //TODO: dit zou moeten werken met meer dan twee elementen in de eyes lijst
					if (eyes.at(0).tl().x < eyes.at(1).br().x && eyes.at(0).br().x > eyes.at(1).tl().x && eyes.at(0).tl().y < eyes.at(1).br().y && eyes.at(0).br().y > eyes.at(1).tl().y){
						int area1 = eyes.at(0).area();
						int area2 = eyes.at(1).area();
						if(area1 > area2){
							eyes.erase(eyes.begin());
						} else {
							eyes.erase(eyes.end());
						}
					}
				}
			}

			for(size_t i=0; i<eyes.size(); ++i){
				cv::Rect r = eyes.at(i);
				cv::Rect newR(r.x*iScale, r.y*iScale, r.width*iScale, r.height*iScale);
				eyes.at(i) = newR;
			}
			scale += 0.1;
		}
		return eyes;
	}

	static cv::vector<cv::Rect> detectMouth(cv::Mat image){
		cv::vector<cv::Rect> rectangles;
		double scale = 0.4;
		while(rectangles.size() == 0 && scale <= 1){
			double iScale = 1/scale;
			cv::Mat image2;
			cv::resize(image, image2, cv::Size(image.cols*scale, image.rows*scale));

			QString file = "/usr/local/share/OpenCV/haarcascades/haarcascade_mcs_mouth.xml";
			int amount = 1;

			cv::CascadeClassifier cc(file.toStdString());
			cc.detectMultiScale(image2, rectangles);

			int param = 5;
			while(rectangles.size() > amount){
				cc.detectMultiScale(image2, rectangles, 1.1, param);
				param += 5;

				cv::Mat image3;
				image2.copyTo(image3);
				for(int i=0; i<rectangles.size(); ++i){
					cv::rectangle(image3, rectangles.at(i), cv::Scalar(0,200,0), 3);
				}

				for(int i=0; i<rectangles.size(); ++i){
					if((rectangles.at(i).br().y)/double(image2.rows)*100 < 75){
						cv::circle(image3, cv::Point(rectangles.at(i).br().x, rectangles.at(i).br().y), 5, cv::Scalar(0, 200, 200), 3);
						cv::rectangle(image3, rectangles.at(i), cv::Scalar(0,0,200), 3);
						rectangles.erase(rectangles.begin()+i);
						--i;
					}
				}

				if(rectangles.size() > 1){
					for(int i=0; i<rectangles.size(); ++i){
						for(int j=0; j<rectangles.size(); ++j){
							if(i != j){
								if (rectangles.at(i).tl().x < rectangles.at(j).br().x && rectangles.at(i).br().x > rectangles.at(j).tl().x && rectangles.at(i).tl().y < rectangles.at(j).br().y && rectangles.at(i).br().y > rectangles.at(j).tl().y){
									int area1 = rectangles.at(i).area();
									int area2 = rectangles.at(j).area();
									if(area1 > area2){
										cv::rectangle(image3, rectangles.at(j), cv::Scalar(200,0, 0), 3);
										rectangles.erase(rectangles.begin()+j);
									} else {
										cv::rectangle(image3, rectangles.at(i), cv::Scalar(200,0,0), 3);
										rectangles.erase(rectangles.begin()+i);
									}
								}
							}
						}
					}
				}

//				cv::imshow("TMP", image3);
//				cv::waitKey();
			}

			for(size_t i=0; i<rectangles.size(); ++i){
				cv::Rect r = rectangles.at(i);
				cv::Rect newR(r.x*iScale, r.y*iScale, r.width*iScale, r.height*iScale);
				rectangles.at(i) = newR;
			}
			scale += 0.1;
			std::cout << "Rescale mouth to " << scale << std::endl;
		}
		return rectangles;
	}

	static cv::vector<cv::Rect> detectNose(cv::Mat image){
		cv::vector<cv::Rect> nose;
		double scale = 0.4;
		while(nose.size() == 0 && scale <= 1){
			double iScale = 1/scale;
			cv::Mat image2;
			cv::resize(image, image2, cv::Size(image.cols*scale, image.rows*scale));
			nose = detect(image2, "/usr/local/share/OpenCV/haarcascades/haarcascade_mcs_nose.xml", 1);

			for(size_t i=0; i<nose.size(); ++i){
				cv::Rect r = nose.at(i);
				cv::Rect newR(r.x*iScale, r.y*iScale, r.width*iScale, r.height*iScale);
				nose.at(i) = newR;
			}
			scale += 0.1;
			std::cout << "Rescale nose to " << scale << std::endl;
		}
		return nose;
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
