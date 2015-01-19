 #include "opencv2/objdetect/objdetect.hpp"
 #include "opencv2/highgui/highgui.hpp"
 #include "opencv2/imgproc/imgproc.hpp"
 #include <sstream>
 #include <iostream>
 #include <stdio.h>

 using namespace std;
 using namespace cv;

 /** Function Headers */
 void detectAndDisplay( Mat frame, vector<Mat> &images );

 /** Global variables */
 String face_cascade_name = "classifier_bin_4/cascade_25.xml";
 CascadeClassifier face_cascade;
 string window_name = "Capture - Face detection";

 int scale     = 3;
 int neighbors = 2;
 int min_row   = 100;
 int min_col   = 100;
 int max_row   = 200 * 4;
 int max_col   = 200 * 4;
 //int r_min     = 65;
 //int r_max     = 90;
 //int b_min     = 100;
 //int b_max     = 170;
 int hist_divider = 1;
 Mat smallImage;
 string fileName;

// Convert type value to a human readable string. Useful for debug
string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

int frameCount = 0;
int main( int argc, const char** argv )
{
   fileName = argv[1];
   VideoCapture cap(argv[1]);
   Mat frame;
   Mat frame_copy;
   vector <Mat> images;


   bool pause = false;

   namedWindow("Parameters", WINDOW_AUTOSIZE);
   createTrackbar ("Scale", "Parameters", &scale, 200, NULL);
   createTrackbar ("Neighbors", "Parameters", &neighbors, 500, NULL);
   createTrackbar ("Min Row", "Parameters", &min_row, 1000, NULL);
   createTrackbar ("Min Col", "Parameters", &min_col, 1000, NULL);
   createTrackbar ("Max Row", "Parameters", &max_row, 1000, NULL);
   createTrackbar ("Max Col", "Parameters", &max_col, 1000, NULL);
   createTrackbar ("Hist Divider", "Parameters", &hist_divider, 16, NULL);

   //-- 1. Load the cascades
   if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

   //-- 2. Read the video stream
   while( true )
{
      if (!pause)
      {
	 cap >> frame;
	 frame_copy = frame.clone();
	 frameCount += 1;
      }
      else
	 frame = frame_copy.clone();
      min_row = frame.rows * 0.057;
      min_col = frame.cols * 0.057;
      //-- 3. Apply the classifier to the frame
      if( !frame.empty() )
      { 
	detectAndDisplay(frame, images);
	//for (size_t index = 0; index < images.size(); index++)
	    //writeImage(images, index, argv[1], frameCount); 
      }
      else
      { printf(" --(!) No captured frame -- Break!"); break; }
      int c = waitKey(5);
      if( c == 'c' ) { break; } // exit

}
return 0;
}

void detectAndDisplay( Mat frame, vector<Mat> &images )
{
  std::vector<Rect> faces;
  Mat frame_gray;
  Mat frame_copy;
  frame_copy = frame.clone();
  images.clear();

  cvtColor( frame, frame_gray, CV_BGR2GRAY );

  //-- Detect faces
  face_cascade.detectMultiScale( frame_gray, faces, 1.05 + scale/100., neighbors, 0|CV_HAAR_SCALE_IMAGE, Size(min_row, min_col), Size(max_row, max_col) );


  for( size_t i = 0; i < faces.size(); i++ )
  {
     // Copy detected image into images[i]
     // Copy from frame_copy so that ID rectangles written onto frame are excluded
	images.push_back(Mat());
	frame_copy(Rect(faces[i].x, faces[i].y, faces[i].width, faces[i].height)).copyTo(images[i]);
	stringstream imName;
	imName << "/home/alon/FRC900_Vision/detection for empty/detected/";
	imName << fileName;
	imName << frameCount;
	imName << i;
	stringstream imNameSmall;
	imNameSmall << imName.str();
	imNameSmall << "_small.png";
	imName << ".png";
	vector<int> compression_params;
    	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
    	compression_params.push_back(9);
	imwrite(imName.str(),images[i],compression_params);
	resize(images[i],smallImage,Size(20,20));
	imwrite(imNameSmall.str(),smallImage,compression_params);
	imshow( window_name, frame );
   }
	cout << "Frame: " << frameCount << " Found:" << faces.size() << endl;
}

