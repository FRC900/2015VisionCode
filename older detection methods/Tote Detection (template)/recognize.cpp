#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

int main() {

Mat input;
Mat logoTemplate;
Mat matchResult;
Mat scaledDown;
namedWindow("Display Image", WINDOW_AUTOSIZE );
namedWindow("Match Result", WINDOW_AUTOSIZE );
logoTemplate = imread("logo.png");

if ( logoTemplate.empty() )
	{
	cout << "no template :(" << endl;
	return -1;
	}

VideoCapture inputVideo(1);
inputVideo.set(CV_CAP_PROP_FRAME_WIDTH,1920);
inputVideo.set(CV_CAP_PROP_FRAME_HEIGHT,1080);
while(1) {
	
	bool openSuccess = inputVideo.read(input);
	if ( !openSuccess )
		{
		cout << "no image data :(";
		break;
		}
	/*//resize(input,scaledDown,0, (scale_ac / scale_deg), (scale_ac / scale_deg), INTER_NEAREST);
	//cout << "webcam image width: " << input.cols << endl;
	//cout << "webcam image height: " << input.rows << endl;
	int result_cols = input.cols - logoTemplate.cols + 1;
	int result_rows = input.rows - logoTemplate.rows + 1;
	//cout << "result columns: " << result_cols << endl;
	//cout << "result rows: " << result_rows << endl;
	matchResult.create( result_cols, result_rows, CV_32FC1);
	matchTemplate(input, logoTemplate, matchResult, CV_TM_SQDIFF_NORMED);
	normalize( matchResult, matchResult, 0, 1, NORM_MINMAX, -1, Mat() );
	double minVal; double maxVal;
	Point minLoc; Point maxLoc;
	minMaxLoc( matchResult, &minVal, &maxVal, &minLoc, &maxLoc);
	Point newMaxLoc = Point( (maxLoc.x * (input.cols / matchResult.cols)), (maxLoc.y * (input.rows / matchResult.rows)));
	cout << "best match location: " << newMaxLoc;
	cout << " match value: " << maxVal << endl;
	circle(input, newMaxLoc, 15, Scalar(0,0,255), -1);
	*/
	rectangle(input, Point((input.cols / 2) - 56, (input.rows / 2) + 25),Point((input.cols / 2) + 56, (input.rows / 2) - 25),Scalar(0,255,0),3);
	imshow("Display Image", input);
	//imshow("Match Result", matchResult);
	if ( waitKey(2) == 27)
		{
		break;
		}
	}
return 0;
}
