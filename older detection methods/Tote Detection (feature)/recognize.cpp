#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include "opencv2/nonfree/features2d.hpp"

using namespace std;
using namespace cv;

Mat input;
Mat logoTemplate;
Mat inputDesc, tempDesc;
vector<KeyPoint> inputKeyPoints, tempKeyPoints;

int main() {

namedWindow("Display Image", WINDOW_AUTOSIZE );
namedWindow("Match Result", WINDOW_AUTOSIZE );

logoTemplate = imread("logo.png");

if ( logoTemplate.empty() )
	{
	cout << "no template :(" << endl;
	return -1;
	}

input = imread("input.png");

if ( input.empty() )
	{
	cout << "no image :(" << endl;
	return -1;
	}
//VideoCapture inputVideo(0);

while(1) {
	
	/*bool openSuccess = inputVideo.read(input);
	if ( !openSuccess )
		{
		cout << "no image data :(";
		break;
		}
	*/
	
	SURFFeatureDetector siftObj;
	siftObj.detect(logoTemplate, tempKeyPoints, tempDesc);
	siftObj.detect(input, inputKeyPoints, inputDesc);
	FlannBasedMatcher matcherObj;
	vector< DMatch > matchesData;
	matcherObj.match(tempDesc, inputDesc, matchesData);
	Mat matchesImage;
	drawMatches(input, inputKeyPoints, logoTemplate, tempKeyPoints, matchesData, matchesImage);
	for ( int i = 0; i < matchesData.size(); i++) {
		cout << "Match found: " << matchesData[i].distance << endl;
	}
	imshow("Match Result", matchesImage);

	if ( waitKey(2) == 27)
		{
		break;
		}
	}
return 0;
}
