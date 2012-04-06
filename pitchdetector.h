#ifndef PITCHDETECTOR_H
#define PITCHDETECTOR_H

#include <QString>

class PitchDetector {
public:
	PitchDetector(bool train);
	PitchDetector(QString retainDir);
	long operator()(QString image);
	long pitch(QString filename);
};

#endif // PITCHDETECTOR_H
