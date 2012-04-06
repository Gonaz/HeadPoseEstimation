#include "pitchdetector.h"
#include <QStringList>

#include <iostream> //TODO debug

PitchDetector::PitchDetector(bool train) {
}

PitchDetector::PitchDetector(QString retainDir){

}

long PitchDetector::operator()(QString image){

}

long PitchDetector::pitch(QString filename){
	filename = filename.split("/").last(); //TODO kan dit niet mooier met fileInfo?
	if(filename.contains("PR")){
		int index = filename.lastIndexOf("PR")+3;
		filename = filename.mid(index, filename.length()-index-6);
		if(filename == "D"){
			return -2;
		} else if(filename == "SD"){
			return -1;
		} else if(filename == "SU"){
			return 1;
		} else if (filename == "U"){
			return 2;
		}
	}

	return 0;
}
