#include "landmarkreader.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <iostream>

LandMarkReader::LandMarkReader(QString filename) : filename(filename){}

QPair<long, long> LandMarkReader::rightEyeCorner(){
	return fetchValue("Outer right eye corner");
}

QPair<long, long> LandMarkReader::innerRightEyeCorner(){
	return fetchValue("Inner right eye corner");
}

QPair<long, long> LandMarkReader::leftEyeCorner(){
	return fetchValue("Outer left eye corner");
}

QPair<long, long> LandMarkReader::innerLeftEyeCorner(){
	return fetchValue("Inner left eye corner");
}

QPair<long, long> LandMarkReader::nose(){
	return fetchValue("Nose tip");
}

QPair<long, long> LandMarkReader::mouthLeftCorner(){
	return fetchValue("Left mouth corner");
}

QPair<long, long> LandMarkReader::mouthRightCorner(){
	return fetchValue("Right mouth corner");
}

QPair<long, long> LandMarkReader::fetchValue(QString name){
	QFile f(filename);
	f.open(QIODevice::ReadOnly);
	QTextStream ts(&f);

	while(ts.readLine().trimmed() != "Labels:"){}
	int count = 0;
	while(ts.readLine().trimmed() != name.trimmed() && !ts.atEnd()){
		++count;
	}

	while(ts.readLine().trimmed() != "2D Image coordinates:" && !ts.atEnd()){}
	int index = 0;
	while(index < count && !ts.atEnd()){
		ts.readLine();
		++index;
	}

	QString resultLine = ts.readLine();
	f.close();

	if(resultLine.trimmed().isEmpty()){
		return qMakePair(long(0), long(0));
	}

	return qMakePair(long(resultLine.split(" ")[0].toDouble()), long(resultLine.split(" ")[1].toDouble()));
}
