#ifndef FACIALNORMAL_H
#define FACIALNORMAL_H

#include <opencv2/core/core.hpp>

int samplePts(QString filename);
void samplePts2(QString filename);
float tilt_angle(float img_facenorm[],float x_vec[],float noselen);
void imgface_normal(float featurePts[],float eulAngle[],cv::Mat frame);
float theta(float img_facenorm[],float eyesMid[],float nosebase[],float facelength);
float slant_angle(float cos_theta,float m1);
void AnglesToAxis(float eulAnlge[], float Rotated_cord[]);
void facialNormal();

#endif //FACIALNORMAL_H
