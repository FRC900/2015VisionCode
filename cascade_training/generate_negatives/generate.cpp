 #include "opencv2/objdetect/objdetect.hpp"
 #include "opencv2/highgui/highgui.hpp"
 #include "opencv2/imgproc/imgproc.hpp"
 #include <sstream>
 #include <iostream>
 #include <stdio.h>

using namespace std;
using namespace cv;

void classifierDetect(CascadeClassifier classifier, Mat frame, int frameNum);

int main(int argc, const char** argv ) {

	string video_path = argv[1];
	string classifier_name = argv[2];
	VideoCapture video_in(video_path);
	Mat frame;

	CascadeClassifier detector;
	if( !detector.load( classifier_name ) ) {
		cout << "Error loading classifier.";
		return -1;
	}
	int frameNum = 0;
	while (true) {
		video_in >> frame;
		frameNum++;
		if(frame.empty()) {
			cout << "No frame! Exiting..." << endl;
			break;
		}
		classifierDetect(detector,frame,frameNum);
	}
	return 0;

}

void classifierDetect(CascadeClassifier classifier,Mat frame, int frameNum) {
	cvtColor(frame,frame,CV_BGR2GRAY);
	vector<Rect> objects;
	classifier.detectMultiScale(frame,objects);
	for(int i = 0; i < objects.size(); i++) {
		Mat subImg = frame(objects[i]);
		Mat sample;
		subImg.copyTo(sample);
		stringstream name;
		name << "./negative_";
		name << frameNum;
		name << "_";
		name << i;
		name << ".png";
		imwrite(name.str(),sample);
	}
}