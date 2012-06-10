#include <QString>
#include <QStringList>
#include <QDir>
#include <vector>
#include <string>
#include <iostream>
#include <limits.h>
#include <typeinfo>

#include "landmarkreader.h"
#include "pitchdetector.h"

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;
using namespace std;

int samplePts(QString filename);
void samplePts2(QString filename);
float tilt_angle(float img_facenorm[],float x_vec[],float noselen);
void imgface_normal(float featurePts[],float eulAngle[],Mat frame);
float theta(float img_facenorm[],float eyesMid[],float nosebase[],float facelength);
float slant_angle(float cos_theta,float m1);
void AnglesToAxis(float eulAnlge[], float Rotated_cord[]);
float featurePts[10];

float tiltangle=0.0;
float eyesMid[2];
float mouthMid[2];
float nosebase[2];
float slantangle=0.0;
//float Rn=0.2;//Ln/Lf
float Rn = 0.557531;
//float Rm=0.45;//Lm/Lf
float Rm=0.538425;
float const PI=3.14159265;
float eulAngle[3];
int b=0;//file index
float Rotated_cord[20];//Rotated Coordinates;
float unitnorm[2];
FILE *fileptr;
std::vector<float> lm;
std::vector<float> ln;
std::vector<float> lf;

struct point{
	template <typename T>
	point(T x, T y) : x(x), y(y) {}
	double x;
	double y;
};

point intersection(point p1, point p2, point p3, point p4) {
	double x1 = p1.x, x2 = p2.x, x3 = p3.x, x4 = p4.x;
	double y1 = p1.y, y2 = p2.y, y3 = p3.y, y4 = p4.y;

	double d = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
	if (d == 0){
//		return NULL;
	}

	// Get the x and y
	double pre = (x1*y2 - y1*x2), post = (x3*y4 - y3*x4);
	double x = ( pre * (x3 - x4) - (x1 - x2) * post ) / d;
	double y = ( pre * (y3 - y4) - (y1 - y2) * post ) / d;

	if ( x < min(x1, x2) || x > max(x1, x2) || x < min(x3, x4) || x > max(x3, x4) ){
//		return NULL;
	}
	if ( y < min(y1, y2) || y > max(y1, y2) || y < min(y3, y4) || y > max(y3, y4) ){
//		return NULL;
	}

	// Return the point of intersection
	point ret = {x, y};
	return ret;
}

float distance(point p1, point p2){
	auto diffX = p1.x - p2.x;
	auto diffY = p1.y - p2.y;

	return sqrt(diffX*diffX + diffY*diffY);
}

int facialNormal() {
	if((fileptr = fopen("output", "w"))==NULL) {//Open file
		printf("Cannot open file.\n");
		exit(1);
	}
//	fprintf(fileptr,"%s        %s       %s        %s\n","File","X-Pitch","Y-Yaw","Z-Roll");
	QString base = "../HeadPoseEstimation/data";
	QString extendedBase = base + "/" + "bs0";

	for(unsigned long i=0; i<20; ++i){
		QString imageDir = extendedBase + QString("%1").arg(i, 2, 10, QChar('0'));
		QStringList images = QDir(imageDir).entryList(QStringList("*.png"));
		foreach(QString image, images){
			if(!image.contains("YR")){
				QString imageFile = imageDir + "/" + image;
				fprintf(fileptr, "%s\n", imageFile.toStdString().c_str());
				fprintf(fileptr, "%d\n", PitchDetector::pitch(imageFile));
				samplePts(imageFile);
				samplePts2(imageFile);
			}
		}
	}
	fclose(fileptr);//close the file

	double div1 = 0;
	for(size_t i=0; i<lm.size(); ++i){
		div1 += (lm[i]/lf[i]);
	}
	std::cout << "lm/lf " << div1/lm.size() << std::endl;

	double div2 = 0;
	for(size_t i=0; i<lf.size(); ++i){
		div2 += (ln[i]/lf[i]);
	}
	std::cout << "ln/lf " << div2/ln.size() << std::endl;

	return 0;
}

void samplePts2(QString filename){
	QString prefix = filename.split("_").first();
	filename = prefix + "_YR_R90_0.png";

	QString landmarksFile = filename;
	landmarksFile.replace(".png", ".lm2");

	LandMarkReader lmr = landmarksFile;
	point tip = {lmr.nose().first, lmr.nose().second};
	point base = {lmr.fetchValue("Right nose peak").first, lmr.fetchValue("Right nose peak").second};

	point eye = {lmr.fetchValue("Outer right eye corner").first, lmr.fetchValue("Outer right eye corner").second};
	point mouth = {lmr.mouthRightCorner().first, lmr.mouthRightCorner().second};

	auto p = intersection(tip, base, eye, mouth);

	ln.push_back(distance(tip, p));
}

int samplePts(QString filename){
	QString landmarksFile = filename;
	landmarksFile.replace(".png", ".lm2");
	Mat image = imread(filename.toStdString());

	LandMarkReader lmr = landmarksFile;
	{//Left Eye Corner cooridinates
		featurePts[0]=lmr.leftEyeCorner().first;
		featurePts[1]=lmr.leftEyeCorner().second;
	}

	{//Right Eye Corner
		featurePts[2]=lmr.rightEyeCorner().first;
		featurePts[3]=lmr.rightEyeCorner().second;
	}

	{//Left Mouth Corner
		featurePts[4]=lmr.mouthLeftCorner().first;
		featurePts[5]=lmr.mouthLeftCorner().second;
	}

	{//Right mouth corner
		featurePts[6]=lmr.mouthRightCorner().first;
		featurePts[7]=lmr.mouthRightCorner().second;
	}

	{//Nose tip
		featurePts[8]=lmr.nose().first;
		featurePts[9]=lmr.nose().second;
	}

	lm.push_back(abs(featurePts[9]-(featurePts[7] + featurePts[5])/2));
	lf.push_back(abs((featurePts[1] + featurePts[3])/2 - (featurePts[5] + featurePts[7])/2));

	imgface_normal(featurePts, eulAngle, image);
//	for(int s=0;s<3;s++,b++){
//		fprintf(fileptr, "%f   ",(180*eulAngle[s])/PI);//Write the Euler angles to a file
//	}
	fprintf(fileptr, "%f\n",(180*eulAngle[1])/PI);
	return 0;
}

void imgface_normal(float featurePts[],float eulAngle [],Mat frame){
	float img_facenorm[2];
	float norm_vec[2];
	float x_vec[2];
	eyesMid[0]=(featurePts[0]+featurePts[2])/2;//X_coordiante
	eyesMid[1]=(featurePts[1]+featurePts[3])/2;//y_cord
	mouthMid[0]=(featurePts[4]+featurePts[6])/2;//x
	mouthMid[1]=(featurePts[5]+featurePts[7])/2;//y

	//Nose base coordiantes
	nosebase[1]=mouthMid[1]-Rm*(mouthMid[1]-eyesMid[1]);
	nosebase[0]=mouthMid[0];

	//Vector along x-axis
	x_vec[0]=featurePts[2]-eyesMid[0];
	x_vec[1]=0;

	//Compute Image face normal from Nose base and Nose tip cordinates
	img_facenorm[0]=featurePts[8]-nosebase[0];//x component of face img normal vector
	img_facenorm[1]=featurePts[9]-nosebase[1];//y component of face img normal vector

	line(frame,Point2f(nosebase[0],nosebase[1]),Point2f(featurePts[8],featurePts[9]),Scalar(255,255,0),1,8,0);
	line(frame,Point2f(nosebase[0],nosebase[1]),Point2f(x_vec[0]+nosebase[0],x_vec[1]+nosebase[1]),Scalar(255,0,255),1,8,0);//x axis
	line(frame,Point2f(nosebase[0],nosebase[1]),Point2f(nosebase[0],eyesMid[1]),Scalar(0,255,255),1,8,0);//y axis

	float facelength=(-eyesMid[1]+mouthMid[1]);
	float noselen=sqrtf((img_facenorm[0]*img_facenorm[0])+(img_facenorm[1]*img_facenorm[1]));

	tiltangle=tilt_angle(img_facenorm,x_vec,noselen);
	slantangle=theta(img_facenorm,eyesMid,nosebase,facelength);

	norm_vec[0]=sin(slantangle)*cos(tiltangle);
	norm_vec[1]=sin(slantangle)*sin(-tiltangle);
	norm_vec[2]=cos(slantangle);

	float normVecmag=sqrtf((norm_vec[0]*norm_vec[0])+(norm_vec[1]*norm_vec[1])+(norm_vec[2]*norm_vec[2]));

	eulAngle[0]=acos(norm_vec[0]/normVecmag)-PI/2;//Euler angle from Normal Vector of Face
	eulAngle[1]=acos(norm_vec[1]/normVecmag)-PI/2;
	eulAngle[2]=acos(norm_vec[2]/normVecmag);

	AnglesToAxis(eulAngle,Rotated_cord);
}

//Compute Tilt angle (Angle b/n image face normal and x-axis (horizontal axis in image plane)
float tilt_angle(float img_facenorm[],float x_vec[],float noselen) {
	float k=sqrtf((x_vec[0]*x_vec[0])+(x_vec[1]*x_vec[1]));
	float cos_tilt=((img_facenorm[0]*x_vec[0])+(img_facenorm[1]*x_vec[1]))/(noselen*k);//cos tilt angle
	tiltangle=acos(cos_tilt);//tilt angle in radian;
	return (tiltangle);
}

//Compute theta(angle b/n Sym axis of face and img face normal)
float theta(float img_facenorm[],float eyesMid[],float nosebase[],float facelength){
	float face_symAxis[]={eyesMid[0]-nosebase[0],eyesMid[1]-nosebase[1]};//along symm axis

	float face_symAxis_mag=sqrtf((face_symAxis[0]*face_symAxis[0])+(face_symAxis[1]*face_symAxis[1]));//magnitude
	float img_facenorm_mag=sqrtf((img_facenorm[0]*img_facenorm[0])+(img_facenorm[1]*img_facenorm[1]));//magnitude imgfacenorm

	float cos_theta=((face_symAxis[0]*img_facenorm[0])+(face_symAxis[1]*img_facenorm[1]))/(face_symAxis_mag*img_facenorm_mag);

	float m1=(img_facenorm_mag*img_facenorm_mag)/(facelength*facelength);//ln/lf

	slantangle=slant_angle(cos_theta, m1);//Slant angle
	return slantangle;
}

//Compute Slant angle(Angle b/n img face normal and optical axis )
float slant_angle(float cos_theta,float m1){
	float m2=cos_theta*cos_theta;
	float a, b, c,dz_sqr;
	a = (Rn*Rn)*(1-m2);
	b = m1+(Rn*Rn)*(2*m2-1);
	c = -Rn*Rn*m2;
	dz_sqr = (-b+sqrtf(b*b-4*a*c))/(2*a);

	slantangle=acos(sqrtf(dz_sqr));// slant angle in radian
	return slantangle;
}

//Compute Sine and Cosine of Euler angles of rotation
void AnglesToAxis(float eulAnlge[], float Rotated_cord[]){
	//Rotation along x(Pitch)
	Rotated_cord[0]=0;//0
	Rotated_cord[1]=cos(eulAngle[0]);//horz y
	Rotated_cord[2]=sin(eulAngle[0]);//horz

	Rotated_cord[3]=0;
	Rotated_cord[4]=sin(eulAngle[0]);//vert z
	Rotated_cord[5]=cos(eulAngle[0]);//Vert
	Rotated_cord[6]=cos(eulAngle[1]);// horz z
	Rotated_cord[7]=0;
	Rotated_cord[8]=sin(eulAngle[1]);//hor

	Rotated_cord[9]=sin(eulAngle[1]);//Vert x
	Rotated_cord[10]=0;
	Rotated_cord[11]=cos(eulAngle[1]);//vert

	//Rotattion along z( Roll)
	Rotated_cord[12]=cos(eulAngle[2]);//horz x
	Rotated_cord[13]=sin(eulAngle[2]);//horz
	Rotated_cord[14]=0;

	Rotated_cord[15]=sin(eulAngle[2]);//Vert y
	Rotated_cord[16]=cos(eulAngle[2]);//Vert
	Rotated_cord[17]=0;
}
