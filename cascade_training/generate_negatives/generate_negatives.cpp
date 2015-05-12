 #include "opencv2/objdetect/objdetect.hpp"
 #include "opencv2/highgui/highgui.hpp"
 #include "opencv2/imgproc/imgproc.hpp"
 #include <sstream>
 #include <iostream>
 #include <stdio.h>

using namespace std;
using namespace cv;

bool save_large = false;
void classifierDetect(CascadeClassifier &classifier, Mat frame, int frameNum);

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

	if(argv[3] == "--save-large") {
		save_large = true;
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

void classifierDetect(CascadeClassifier &classifier, Mat frame, int frameNum) {
	cvtColor(frame,frame,CV_BGR2GRAY);
	vector<Rect> objects;
	equalizeHist(frame,frame);
	classifier.detectMultiScale(frame,objects);
	for(int i = 0; i < objects.size(); i++) {
		Mat subImg = frame(objects[i]);
		Mat sample;
		subImg.copyTo(sample);
		stringstream name;
		stringstream name_small;
		Mat sample_small;
		name << "./negative_";
		name << frameNum;
		name << "_";
		name << i;
		name_small << name;
		name_small << "-s";
		name << ".png";
		name_small << ".png";
		resize(sample,sample_small,Size(20,20));
		imwrite(name_small.str(),sample_small);
		if(save_large) {
			imwrite(name.str(),sample);
		}
	}
}