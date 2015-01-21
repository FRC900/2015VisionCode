#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

const double DETECT_ASPECT_RATIO = 1.0;
/** Function Headers */
void detectAndDisplay(Mat frame, const vector<Rect> &rect, vector<Mat> &images );
void generateThreshold(const Mat &ImageIn, Mat &ImageOut);

/** Global variables */
String face_cascade_name = "../cascade_training/classifier_bin_5/cascade_16.xml";
//String face_cascade_name = "classifier_bin_6/cascade_11.xml";
CascadeClassifier face_cascade;
string window_name = "Capture - Face detection";

int scale     = 3;
int neighbors = 2;
int minDetectSize   = 20;
int maxDetectSize   = 200 * 4;
//int r_min     = 65;
//int r_max     = 90;
//int b_min     = 100;
//int b_max     = 170;
int hist_divider = 1;

int H_MIN =  53;
int H_MAX =  97;
int S_MIN =  30;
int S_MAX = 185;
int V_MIN =  57;
int V_MAX = 184;

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

void writeImage(const vector<Mat> &images, size_t index, const char *path, int frameCount)
{
   if (index < images.size())
   {
      // Create filename, save image
      stringstream fn;
      fn << path;
      fn << "_";
      fn << frameCount;
      fn << "_";
      fn << index;
      imwrite(fn.str().substr(fn.str().rfind('\\')+1) + ".png", images[index]);

      // Save grayscale equalized version
      Mat frameGray;
      cvtColor( images[index], frameGray, CV_BGR2GRAY );
      equalizeHist( frameGray, frameGray );
      imwrite(fn.str().substr(fn.str().rfind('\\')+1) + "_g.png", frameGray);

      // Save 20x20 version of the same image
      Mat smallImg;
      resize(images[index], smallImg, Size(20,20));
      imwrite(fn.str().substr(fn.str().rfind('\\')+1) + "_s.png", smallImg);

      // Save grayscale equalized version of small image
      cvtColor( smallImg, frameGray, CV_BGR2GRAY );
      equalizeHist( frameGray, frameGray );
      imwrite(fn.str().substr(fn.str().rfind('\\')+1) + "_g_s.png", frameGray);
   }
}

int main( int argc, const char** argv )
{
   string capPath;
   VideoCapture cap;
   if (argc < 2)
   {
      cap = VideoCapture(0);
      capPath = "negative/1-21";
   }
   else if (isdigit(*argv[1]))
   {
      cap = VideoCapture(*argv[1] - '0');
      capPath = "negative/1-21" + (*argv[1] - '0');
   }
   else
   {
      cap = VideoCapture(argv[1]);
      capPath = "negative/" + string(argv[1]).substr(string(argv[1]).rfind('/')+1);
   }
   cerr << capPath << endl;

   Mat frame;
   Mat frame_copy;
   vector <Mat> images;
   int frameCount = 0;
   	
   bool pause = false;
   bool captureAll = false;
   
   namedWindow("Parameters", WINDOW_AUTOSIZE);
   createTrackbar ("Scale", "Parameters", &scale, 50, NULL);
   createTrackbar ("Neighbors", "Parameters", &neighbors, 50, NULL);
   createTrackbar ("Min Detect", "Parameters", &minDetectSize, 1000, NULL);
   createTrackbar ("Max Detect", "Parameters", &maxDetectSize, 1000, NULL);
   //createTrackbar ("R Min", "Parameters", &r_min, 256, NULL);
   //createTrackbar ("R Max", "Parameters", &r_max, 256, NULL);
   //createTrackbar ("B Min", "Parameters", &b_min, 256, NULL);
   //createTrackbar ("B Max", "Parameters", &b_max, 256, NULL);
   //createTrackbar ("Hist Divider", "Parameters", &hist_divider, 16, NULL);
   string trackbarWindowName = "HSV";
   namedWindow(trackbarWindowName, WINDOW_AUTOSIZE);
   createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, 179, NULL);
   createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, 179, NULL);
   createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, 255, NULL);
   createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, 255, NULL);
   createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, 255, NULL);
   createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, 255, NULL);

   //-- 1. Load the cascades
   if( !face_cascade.load( face_cascade_name ) )
   {
      cerr << "--(!)Error loading " << face_cascade_name << endl; 
      return -1; 
   }

   //-- 2. Read the video stream
   while( true )
   {
      if (!pause)
      {
	 cap >> frame;
	 if( frame.empty() )
	 { 
	    printf(" --(!) No captured frame -- Break!\n");
	    exit(0); 
	 }
	 pyrDown(frame, frame);
	 frame_copy = frame.clone();
	 frameCount += 1;
      }
      else
	 frame = frame_copy.clone();
         
      minDetectSize = frame.cols * 0.057;

      //-- 3. Apply the classifier to the frame

      // Threshold the image using supplied HSV value
      Mat frameThresh;
      vector<Mat> splitFrame;
      vector < vector <Point> > contours;
      vector < Rect > rects;

      generateThreshold(frame, frameThresh);
      imshow("Threshold", frameThresh);
      findContours(frameThresh, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

      // Find contours in the thresholded image.  Grab bounding
      // rectangles around those. Ideally, if the HSV values are 
      // set correctly, these rectangles will be the only
      // places that green-enough objects are found
      for (size_t i = 0; i < contours.size(); i++)
      {
	 Rect rect = boundingRect(contours[i]);
	 // Scale up rectangle a bit so that the
	 // bounds are larger than each green blob detected
	 const double RECT_SCALE_UP = 0.33
	 rect.x -= rect.height / (1/(RECT_SCALE_UP/2));
	 rect.y -= rect.width / (1/(RECT_SCALE_UP/2));
	 rect.height *= (1.0 + RECT_SCALE_UP);
	 rect.width *= (1.0 + RECT_SCALE_UP);
	 // Save bounding rects which are large enough
	 // to actually contain the smallest image 
	 // searched for
	 if ((rect.width > (minDetectSize * DETECT_ASPECT_RATIO))&&(rect.height > minDetectSize))
	 {
	    rects.push_back(rect);
	 }
      }

      detectAndDisplay( frame, rects, images ); 

      char c = waitKey(5);
      if( c == 'c' ) { break; } // exit
      else if( c == ' ') { pause = !pause; }
      else if( c == 'f')  // advance to next frame
      {
	 cap >> frame;
	 if( frame.empty() )
	 { printf(" --(!) No captured frame -- Break!\n"); exit(0); }
	 pyrDown(frame, frame);
	 frame_copy = frame.clone();
	 frameCount += 1;
      }
      else if (c == 'A') // toggle capture-all
      {
	 captureAll = !captureAll;
      }
      else if (c == 'a') // save all detected images
      {
	 for (size_t index = 0; index < images.size(); index++)
	    writeImage(images, index, capPath.c_str(), frameCount);
      }
      else if (isdigit(c)) // save a single detected image
      {
	 writeImage(images, c - '0', capPath.c_str(), frameCount);
      }
      for (size_t index = 0; captureAll && (index < images.size()); index++)
	 writeImage(images, index, capPath.c_str(), frameCount);
   }
   return 0;
}

float range[] = { 0, 256 } ;
const float* histRange = { range };
bool uniform = true; bool accumulate = false;
/** @function detectAndDisplay */
void detectAndDisplay( Mat frame, const vector<Rect> &rects, vector<Mat> &images )
{
  std::vector<Rect> faces;
  Mat frame_gray;
  Mat frame_copy;

  frame_copy = frame.clone();
  images.clear();
  int histSize = 256 / (hist_divider ? hist_divider : 1);

  cvtColor( frame, frame_gray, CV_BGR2GRAY );
  equalizeHist( frame_gray, frame_gray );

  //-- Detect faces
  face_cascade.detectMultiScale( frame_gray, faces, 1.05 + scale/100., neighbors, 0|CV_HAAR_SCALE_IMAGE, Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize), Size(maxDetectSize * DETECT_ASPECT_RATIO, maxDetectSize) );

  // Mark and save up to 10 detected images
  for( size_t i = 0; i < min(faces.size(), (size_t)10); i++ )
  {
     // Copy detected image into images[i]
     // Copy from frame_copy so that ID rectangles written onto frame are excluded
     images.push_back(Mat());
     frame_copy(Rect(faces[i].x, faces[i].y, faces[i].width, faces[i].height)).copyTo(images[i]);

     // Split into individual B,G,R channels so we can run a histogram on each
     vector<Mat> bgr_planes;
     split (images[i], bgr_planes);
     bool uniform = true; bool accumulate = false;

     //cvtColor(images[i], images[i], COLOR_BGR2HSV);
     Mat hist[3];
     double min_val[3];
     double max_val[3];
     //int min_idx[3];
     int max_idx[3];

     for (size_t j = 0; j < 3; j++)
     {
	/// Compute the histograms:
	calcHist( &bgr_planes[j], 1, 0, Mat(), hist[j], 1, &histSize, &histRange, uniform, accumulate );

	// Remove 0 & 255 intensities since these seem to confuse things later
	hist[j].at<float>(255) = 0.;
	hist[j].at<float>(0) = 0.;

	// Grab the color intensity peak
	Point min, max;
	minMaxLoc(hist[j], min_val, max_val + j, &min, &max);
	//min_idx[j] = min.y;
	max_idx[j] = max.y;
     }

     //cerr << i << " " << max_idx[0] << " " << max_idx[1] << " " <<max_idx[2] << " " << images[i].cols <<  " " << images[i].rows << " " << type2str(images[i].type()) << endl;
#if 0
     if ((max_idx[0] + max_idx[1]) < max_idx[2])
     {
	cout << "R hit " ;
	for (size_t j = 0; j < 3; j++)
	   cout << " " << max_idx[j];
	cout << endl;
	rectangle( frame, faces[i], Scalar( 0, 0, 255 ),3);
     }
     else if ((max_idx[1] + max_idx[2]) < max_idx[0])
     {
	cout << "B hit " ;
	for (size_t j = 0; j < 3; j++)
	   cout << " " << max_idx[j];
	cout << endl;
	rectangle( frame, faces[i], Scalar( 255, 0, 0 ),3);
     }
     else if ((std::min(max_idx[0], max_idx[1]) * 1.5) < max_idx[2])    
     {
	cout << "R2 hit " ;
	for (size_t j = 0; j < 3; j++)
	   cout << " " << max_idx[j];
	cout << endl;
	rectangle( frame, faces[i], Scalar( 0, 128, 255 ),3);
     }
     else if ((std::min(max_idx[1], max_idx[2]) * 1.5) < max_idx[0])    
     {
	cout << "B2 hit " ;
	for (size_t j = 0; j < 3; j++)
	   cout << " " << max_idx[j];
	cout << endl;
	rectangle( frame, faces[i], Scalar( 255, 128, 0 ),3);
     }
     else
#endif
	// Hightlight detected images which are fully contained in 
	// green contour bounding rectangles
	bool inRect = false;
	for (size_t j = 0 ; !inRect && (j < rects.size()); j++)
	{
	   Rect intersect = faces[i] & rects[j];
	   if (intersect == faces[i])
	      inRect = true;
	}
	if (inRect)
	   rectangle( frame, faces[i], Scalar( 0, 0, 255 ),3);
	else
	   rectangle( frame, faces[i], Scalar( 255, 0, 255 ),3);

     // Label each outlined image with a digit.  Top-level code allows
     // users to save these small images by hitting the key they're labeled with
     // This should be a quick way to grab lots of falsly detected images
     // which need to be added to the negative list for the next
     // pass of classifier training.
     stringstream label;
     label << i;
     putText( frame, label.str(), Point(faces[i].x, faces[i].y), FONT_HERSHEY_PLAIN, 1.0, Scalar(255, 255, 0));
#if 0
     // Draw the histograms for B, G and R
     int hist_w = 512; int hist_h = 400;
     int bin_w = cvRound( (double) hist_w/histSize );

     Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
     /// Normalize the result to [ 0, histImage.rows ]
     for (size_t j = 0; j < 3; j++)
	normalize(hist[j], hist[j], 0, histImage.rows, NORM_MINMAX, -1, Mat() );

     const Scalar colors[3] = {Scalar(255,0,0), Scalar(0,255,0), Scalar(0,0,255)};
     // For each point in the histogram
     for( int ii = 1; ii < histSize; ii++ )
     {
	// Draw for each channel
	for (size_t jj = 0; jj < 3; jj++)
	   line( histImage, Point( bin_w*(ii-1), hist_h - cvRound(hist[jj].at<float>(ii-1)) ) ,
		 Point( bin_w*(ii), hist_h - cvRound(hist[jj].at<float>(ii)) ),
		 colors[jj], 2, 8, 0  );
     }

     // Display detected images and histograms of those images
     stringstream winName;
     winName << "Face "; 
     winName << i;
     namedWindow(winName.str(), CV_WINDOW_AUTOSIZE);
     imshow (winName.str(), images[i]);
     resizeWindow(winName.str(), images[i].cols, images[i].rows);
     winName << " Hist";
     imshow (winName.str(), histImage);
#endif
  }
#if 0
  for( size_t i = 0; i < faces.size(); i++ )
  {
     //Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
     rectangle( frame, faces[i], Scalar( 255, 0, 255 ),3);
  }
#endif
  for (size_t i = 0; i < rects.size(); i++)
  {
     Rect rect = rects[i];
     rectangle (frame, rect, Scalar(255,255,0), 3);
  }

  //-- Show what you got
  imshow( window_name, frame );
}

// Take an input image. Threshold it so that pixels within
// the HSV range specified by [HSV]_[MIN,MAX] are set to non-zero
// and the rest of the image is set to zero. Apply a morph
// open to the resulting image
void generateThreshold(const Mat &ImageIn, Mat &ImageOut)
{
   Mat ThresholdLocalImage;
   vector<Mat> SplitImage;
   Mat SplitImageLE;
   Mat SplitImageGE;

   cvtColor(ImageIn, ThresholdLocalImage, CV_BGR2HSV, 0);
   split(ThresholdLocalImage, SplitImage);
   int max[3] = {H_MAX, S_MAX, V_MAX};
   int min[3] = {H_MIN, S_MIN, V_MIN};
   for (size_t i = 0; i < SplitImage.size(); i++)
   {
      compare(SplitImage[i], min[i], SplitImageGE, cv::CMP_GE);
      compare(SplitImage[i], max[i], SplitImageLE, cv::CMP_LE);
      bitwise_and(SplitImageGE, SplitImageLE, SplitImage[i]);
   }
   bitwise_and(SplitImage[0], SplitImage[1], ImageOut);
   bitwise_and(SplitImage[2], ImageOut, ImageOut);

   Mat erodeElement (getStructuringElement( MORPH_RECT,Size(3,3)));
   //dilate with larger element to make sure object is nicely visible
   Mat dilateElement(getStructuringElement( MORPH_ELLIPSE,Size(11,11)));
   erode(ImageOut, ImageOut, erodeElement, Point(-1,-1), 2);
   dilate(ImageOut, ImageOut, dilateElement, Point(-1,-1), 2);
}
