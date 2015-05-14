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

int indexNeg = 1;
int negative_count = 0;

int main(int argc, const char* argv[] ) {
	string video_path = argv[1];
	string classifier_name = argv[2];
	VideoCapture video_in(video_path);
	Mat frame;

	CascadeClassifier detector;
	if( !detector.load( classifier_name ) ) {
		cout << "Error loading classifier.";
		return -1;
	}
	for(int i = 3; i < argc; i++) { //process some arguments
		if(argv[i] == "--save-large") {
			save_large = true;
		}
		if(argv[i] == "-negative_count") {
			negative_count = atoi(argv[i+1]);
			i++;
		}
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
		cout << "Frame " << frameNum << " completed" << endl;
		if(indexNeg == negative_count) {
			break;
		}
	}
	cout << "Generated " << indexNeg << " negatives. Exiting" << endl;
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
		Mat sample_small;
		name << "./negatives/img_";
		name << indexNeg;
		name << ".png";
		resize(sample,sample_small,Size(20,20));
		imwrite(name.str(),sample_small);
		if(save_large) {
			stringstream name_large;
			name_large << "./negatives/img-l_";
			name_large << indexNeg;
			name_large << ".png";
			imwrite(name_large.str(),sample);
		}
		if(indexNeg == negative_count){
			break;
		}
		indexNeg++;
	}
}