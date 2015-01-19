#include <iostream>
#include <stdio.h>
#include <string>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/nonfree/features2d.hpp"
#include <opencv2/core/core.hpp>


using namespace std;
using namespace cv;

Mat input;
Mat logoTemplate;
Mat blurInput;
Mat greenImage;
Mat redImage;
Mat redAndGreen;

int MinBlueRed = 195;
int MinGreenRed = 195;
int MinRedRed = 195;
int MaxBlueRed = 255;
int MaxGreenRed = 255;
int MaxRedRed = 255;
int BlueScale = 1;
int GreenScale = 1;
int RedScale = 1;

Scalar MinGreen;
Scalar MaxGreen;
Scalar MinRed;
Scalar MaxRed;

vector < vector <Point> > finalImageContours;
vector < vector <Point> > greenImageContours;
vector <Point> targetContour;

void allTrackbars() {
namedWindow("Original", WINDOW_NORMAL);
namedWindow("Result", WINDOW_NORMAL );
namedWindow("Found Logo", WINDOW_NORMAL);
namedWindow("Red Parameters",WINDOW_NORMAL);
namedWindow("Green",WINDOW_NORMAL);
namedWindow("White",WINDOW_NORMAL);
namedWindow("Scales",WINDOW_NORMAL);

createTrackbar("Min Blue","Red Parameters", &MinBlueRed,255);
createTrackbar("Min Green","Red Parameters", &MinGreenRed,255);
createTrackbar("Min Red","Red Parameters", &MinRedRed,255);

createTrackbar("Max Blue","Red Parameters", &MaxBlueRed,255);
createTrackbar("Max Green","Red Parameters", &MaxGreenRed,255);
createTrackbar("Max Red","Red Parameters", &MaxRedRed,255);

createTrackbar("Blue Scale","Scales", &BlueScale,200);
createTrackbar("Green Scale","Scales", &GreenScale,200);
createTrackbar("Red Scale","Scales", &RedScale,200);
}

vector <Point> largestContour ( vector < vector <Point> > contoursList) {
	vector <Point> largestContour = contoursList[0];
	for (int i = 0; i < contoursList.size(); i++){
		if (contourArea(contoursList[i]) > contourArea(largestContour))
		largestContour = contoursList[i];	
		}
	return largestContour;
}


void drawContour (Mat &destImage, vector <Point> contour, Scalar color, int thickness) {
	//create vector of contours to store one value
	vector < vector <Point> > fakeContour;
	// add passed value to vector
	fakeContour.push_back(contour);
	//draw "all" contours in vector
	drawContours(destImage,fakeContour,-1,color,thickness);
}
void dilateAndErode (Mat &Image) {

Mat dilateElement = getStructuringElement(MORPH_RECT,Size(2,2));
Mat erodeElement = getStructuringElement(MORPH_RECT,Size(2,2));

dilate(Image,Image,dilateElement,Point(-1,-1),1);
erode(Image,Image,erodeElement,Point(-1,-1),4);
dilate(Image,Image,dilateElement,Point(-1,-1),1);
}

void framesPerSecond(int counter,clock_t startTime) {
//timer stuff to print fps
clock_t t;
t = clock() - startTime;
double secondT = (float(t))/CLOCKS_PER_SEC;
//cout << 1 / secondT << endl;
}
int counter = 0;
int main(int argc, char* argv[]) {

allTrackbars();
VideoCapture inputVideo(0);
while(1) {
	clock_t start = clock();
	/*bool openSuccess = inputVideo.read(input);
	if ( !openSuccess )
		{
		cout << "no image data :(";
		break;
		}*/

	if (argv[1] == NULL) {
		//cout << "Please enter image name" << endl;
		//return -1;
		argv[1] = "bin1.jpg";
		}
	
	input = imread(argv[1]);
	if ( input.empty() )
		{
		cout << "no image" << endl;
		return -1;
		}
	uchar depth;
	imshow("Original",input);
	blur(input,blurInput,Size(2,2));
	Mat splitImage[3];
	//trying a new method, split the images and (green - (red + blue))
	split(blurInput, splitImage);
	add(splitImage[0],BlueScale, splitImage[0]);
	add(splitImage[1],GreenScale,splitImage[1]);
	add(splitImage[2],RedScale,splitImage[2]);
	Mat redAndBlue;
	add(splitImage[0], splitImage[2], redAndBlue);
	subtract(splitImage[1],redAndBlue, greenImage);
	dilate(greenImage,greenImage,getStructuringElement(MORPH_RECT,Size(1,1)),Point(-1,-1),2);
	//find green blobs
	findContours(greenImage,greenImageContours,CV_RETR_EXTERNAL,CV_CHAIN_APPROX_SIMPLE);
	//fill in blobs
	drawContours(greenImage,greenImageContours,-1,Scalar(255,255,255),CV_FILLED);
	//find white blobs
	inRange(blurInput,MinRed,MaxRed,redImage);
	//dilate green image to merge logo into one blob
	imshow("Green", greenImage);
	imshow("White", redImage);
	//create AND of green and white images
	bitwise_and(greenImage,redImage,redAndGreen);
	dilateAndErode(redAndGreen);
	imshow("Found Logo", redAndGreen);
	//find logo
	findContours(redAndGreen,finalImageContours,CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);
	//find largest contour (script)
	if (finalImageContours.size() != 0) {
	targetContour = largestContour(finalImageContours);
	//draw contour (script)
	drawContour(input,targetContour,Scalar(0,255,0),5);
	}else {
	cout << "No contours found" << endl;
	}
	imshow("Result", input);
	//wait x ms for esc key pressed
	if ( waitKey(3) == 27)
		{
		break;
		}
	counter++;
	framesPerSecond(counter,start);
	}
return 0;
}
