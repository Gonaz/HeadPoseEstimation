#ifndef LANDMARKREADER_H
#define LANDMARKREADER_H

#include <QPair>

class LandMarkReader {
public:
	LandMarkReader(QString filename);
	QPair<long, long> rightEyeCorner();
	QPair<long, long> leftEyeCorner();
	QPair<long, long> nose();
	QPair<long, long> mouthLeftCorner();
	QPair<long, long> mouthRightCorner();
	QPair<long, long> innerLeftEyeCorner();
	QPair<long, long> innerRightEyeCorner();

	QPair<long, long> fetchValue(QString name); //TODO: dit staat tijdelijk public
private:
	QString filename;
};

#endif // LANDMARKREADER_H
