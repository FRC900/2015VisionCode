#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <opencv2/opencv.hpp>

#include <iostream>
#include <stdio.h>

#include "imagedetect.hpp"
#include "videoin_c920.hpp"

// Define this to threshold image by color in addition to 
// checking for cascade classifier detection
//#define USE_THRESHOLD

// Define this to threshold image by histogram intensity
// #define USE_HISTOGRAM

using namespace std;
using namespace cv;

int H_MIN =  53;
int H_MAX =  97;
int S_MIN =  30;
int S_MAX = 185;
int V_MIN =  57;
int V_MAX = 184;

string windowName = "Capture - Face detection";

int histIgnoreMin = 14;
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

// For a given frame, find the pixels values for each channel with 
// the highest intensities
void generateHistogram(const Mat &frame, double *minIdx, double *maxIdx)
{
   int histSize = 256 / (histDivider ? histDivider : 1);
   float range[] = { 0, 256 } ;
   const float* histRange = { range };
   bool uniform = true, accumulate = false;

   // Split into individual B,G,R channels so we can run a histogram on each
   vector<Mat> bgrPlanes;
   split (frame, bgrPlanes);

   //cvtColor(images[i], images[i], COLOR_BGR2HSV);
   Mat hist[3];
   double minVal[3];
   double maxVal[3];

   for (size_t i = 0; i < 3; i++)
   {
      /// Compute the histograms:
      calcHist(&bgrPlanes[i], 1, 0, Mat(), 
	    hist[i], 1, &histSize, &histRange, uniform, accumulate );

      // Remove 0 & 255 intensities since these seem to confuse things later
      hist[i].at<float>(0)   = 0.;
      hist[i].at<float>(255) = 0.;

      // Grab the color intensity peak
      Point min, max;
      minMaxLoc(hist[i], minVal + i, maxVal + i, &min, &max);
      minIdx[i] = min.y;
      maxIdx[i] = max.y;
   }
}

void writeImage(const vector<Mat> &images, size_t index, const char *path, int frameCounter)
{
   if (index < images.size())
   {
      // Create filename, save image
      stringstream fn;
      fn << path;
      fn << "_";
      fn << frameCounter;
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
   VideoIn *cap;
   const size_t detectMax = 10;
   const string frameOpt = "--frame=";
   double frameStart = 0.0;
   const size_t distanceListLength = 10;
   float distanceVal;
   float distanceList[distanceListLength] = {0};
   if (argc < 2)
   {
      // No arguments? Open default camera
      // and hope for the best
      cap = new VideoIn(0);
      capPath = "negative/2-03";
   }
   else 
   {
      // Read through command line args, extract
      // cmd line parameters and input filename
      int fileArgc;
      for (fileArgc = 1; fileArgc < argc; fileArgc++)
      {
	 if (frameOpt.compare(0, frameOpt.length(), argv[fileArgc], frameOpt.length()) == 0)
	    frameStart = (double)atoi(argv[fileArgc] + frameOpt.length());
	 else
	    break;
      }
      // Digit? Open camera
      if (isdigit(*argv[fileArgc]))
      {
	 cap = new VideoIn(*argv[fileArgc] - '0');
	 capPath = "negative/2-03_" + (*argv[fileArgc] - '0');
      }
      else
      {
	 // Open file name
	 cap = new VideoIn(argv[fileArgc]);
	 if (cap->VideoCap())
	 {
	    cap->VideoCap()->set(CV_CAP_PROP_POS_FRAMES, frameStart);
	    cap->frameCounter(frameStart);
	 }
	 capPath = "negative/" + string(argv[fileArgc]).substr(string(argv[fileArgc]).rfind('/')+1);
      }
   }

   Mat frame;
   vector <Mat> images;
   	
   bool pause = false;
   bool captureAll = false;
   
   namedWindow("Parameters", WINDOW_AUTOSIZE);
   createTrackbar ("Scale", "Parameters", &scale, 50, NULL);
   createTrackbar ("Neighbors", "Parameters", &neighbors, 50, NULL);
#ifdef USE_HISTOGRAM
   createTrackbar ("Hist Thresh", "Parameters", &histIgnoreMin, 40, NULL);
#endif
   //createTrackbar ("Min Detect", "Parameters", &minDetectSize, 1000, NULL);
   createTrackbar ("Max Detect", "Parameters", &maxDetectSize, 1000, NULL);
   //createTrackbar ("R Min", "Parameters", &r_min, 256, NULL);
   //createTrackbar ("R Max", "Parameters", &r_max, 256, NULL);
   //createTrackbar ("B Min", "Parameters", &b_min, 256, NULL);
   //createTrackbar ("B Max", "Parameters", &b_max, 256, NULL);

#ifdef USE_THRESHOLD
   string trackbarWindowName = "HSV";
   namedWindow(trackbarWindowName, WINDOW_AUTOSIZE);
   createTrackbar( "H_MIN", trackbarWindowName, &H_MIN, 179, NULL);
   createTrackbar( "H_MAX", trackbarWindowName, &H_MAX, 179, NULL);
   createTrackbar( "S_MIN", trackbarWindowName, &S_MIN, 255, NULL);
   createTrackbar( "S_MAX", trackbarWindowName, &S_MAX, 255, NULL);
   createTrackbar( "V_MIN", trackbarWindowName, &V_MIN, 255, NULL);
   createTrackbar( "V_MAX", trackbarWindowName, &V_MAX, 255, NULL);
#endif

   const char *cascadeName = "../cascade_training/classifier_bin_6/cascade_oldformat_30.xml";
   // Use GPU code if hardware is detected, otherwise
   // fall back to CPU code
   BaseCascadeDetect *detectCascade;
   if (gpu::getCudaEnabledDeviceCount() > 0)
      detectCascade = new GPU_CascadeDetect(cascadeName);
   else
      detectCascade = new CPU_CascadeDetect(cascadeName);

   // Load the cascades
   if( !detectCascade->loaded() )
   {
      cerr << "--(!)Error loading " << cascadeName << endl; 
      return -1; 
   }

   // Read the video stream
   int startFrame = cap->frameCounter() + 1;
   while(cap->getNextFrame(pause, frame))
   {
      // Minimum size of a bin at ~30 feet distance
      minDetectSize = frame.cols * 0.057;

#ifdef USE_THRESHOLD
      // Threshold the image using supplied HSV value
      Mat frameThresh;
      vector<Rect> threshRects;
      thresholdImage(frame, frameThresh, threshRects,
		H_MIN, H_MAX, S_MIN, S_MAX, V_MIN, V_MAX);
      imshow("Threshold", frameThresh);
#endif

      // Apply the classifier to the frame
      vector<Rect> detectRects;
      vector<unsigned> detectDirections;
      detectCascade->cascadeDetect(frame, detectRects, detectDirections); 

      vector<Rect> passedHistFilterRects;
      vector<unsigned> passedHistFilterDirections;
      images.clear();
      // Mark and save up to detectMax detected images
      for( size_t i = 0; i < min(detectRects.size(), detectMax); i++ )  
      {
#ifdef USE_HISTOGRAM
	 // ignore really dim images - ones where the peak intensity of each
	 // channel is below histIgnoreMin
	 double minIdx[3];
	 double maxIdx[3];
	 generateHistogram(frame(Rect(detectRects[i].x, detectRects[i].y, detectRects[i].width, detectRects[i].height)), minIdx, maxIdx);
	 //cerr << i << " " << maxIdx[0] << " " << maxIdx[1] << " " << maxIdx[2] << endl;
	 if ((maxIdx[0] > histIgnoreMin) || 
	     (maxIdx[1] > histIgnoreMin) || 
	     (maxIdx[2] > histIgnoreMin))
#endif
	 {
	    // Copy detected image into images[i]
	    passedHistFilterRects.push_back(detectRects[i]);
	    passedHistFilterDirections.push_back(detectDirections[i]);
	    images.push_back(frame(detectRects[i]).clone());
	 }
      }
      
      // Filter out images using threshold values - 
      // since bins are green this could be used as a second pass
      // to get rid of false positives which aren't green enough
      vector <Rect> filteredRects;
#ifdef USE_THRESHOLD
      filterUsingThreshold(passedHistFilterRects, threshRects, filteredRects);
#endif

      int filterIdx = 0;
      for( size_t i = 0; i < min(passedHistFilterRects.size(), detectMax); i++ )  
      {
	 // Hightlight detected images which are fully contained in 
	 // green contour bounding rectangles
	 bool inRect = false;
	 if (filteredRects.size() && 
	     (filteredRects[filterIdx] == passedHistFilterRects[i]))
	 {
	    inRect = true;
	    filterIdx += 1;
	 }
	 Scalar rectColor;
	 switch (passedHistFilterDirections[i])
	 {
	    case 1:
	       rectColor = Scalar(0,0,255);
	       break;
	    case 2:
	       rectColor = Scalar(0,255,0);
	       break;
	    case 4:
	       rectColor = Scalar(255,0,0);
	       break;
	    case 8:
	       rectColor = Scalar(255,255,0);
	       break;
	    default:
	       rectColor = Scalar(255,0,255);
	       break;
	 }

	 rectangle( frame, passedHistFilterRects[i], rectColor, 3);
	 float FOVFrac = (float)passedHistFilterRects[i].width / (float)frame.cols;
	 float totalFOV = 12.0 / FOVFrac;
	 distanceVal = totalFOV / tan(0.59);
	 // Label each outlined image with a digit.  Top-level code allows
	 // users to save these small images by hitting the key they're labeled with
	 // This should be a quick way to grab lots of falsly detected images
	 // which need to be added to the negative list for the next
	 // pass of classifier training.
	 stringstream label;
	 label << i;
	 putText( frame, label.str(), 
	       Point(passedHistFilterRects[i].x, passedHistFilterRects[i].y), 
	       FONT_HERSHEY_PLAIN, 2.0, Scalar(255, 0, 255));
      }
      distanceList[(cap->frameCounter() - startFrame) % distanceListLength] = distanceVal;
      if ((cap->frameCounter() - startFrame) >= distanceListLength) {
	 float sum = 0.0;
	 float sumSquare = 0.0;
	 for (size_t i = 0; i < distanceListLength; i++)
	    sum = sum + distanceList[i];
	 float average = sum / distanceListLength;
	 for (size_t i = 0; i < distanceListLength; i++)
	    sumSquare = (distanceList[i] - average) * (distanceList[i] - average) + sumSquare;
	 float stdev = (float)sumSquare / (float)distanceListLength;
	 cout << "Average: " << average << " Standard Deviation: " << stdev << endl;
      }
      //for (size_t i = 0; i < threshRects.size(); i++)
      //rectangle (frame, threshRects[i], Scalar(255,255,0), 3);

      if (captureAll)
	 putText( frame, "A", Point(25,25),
	       FONT_HERSHEY_PLAIN, 2.5, Scalar(0, 255, 255));

      //-- Show what you got
      imshow( windowName, frame );

      char c = waitKey(5);
      if( c == 'c' ) { break; } // exit
      else if( c == ' ') { pause = !pause; }
      else if( c == 'f')  // advance to next frame
      {
	 cap->getNextFrame(false, frame);
      }
      else if (c == 'A') // toggle capture-all
      {
	 captureAll = !captureAll;
      }
      else if (c == 'a') // save all detected images
      {
	 for (size_t index = 0; index < images.size(); index++)
	    writeImage(images, index, capPath.c_str(), cap->frameCounter());
      }
      else if (c == 'p')
      {
	 cout << cap->frameCounter() << endl;
      }
      else if (isdigit(c)) // save a single detected image
      {
	 writeImage(images, c - '0', capPath.c_str(), cap->frameCounter());
      }
      for (size_t index = 0; captureAll && (index < images.size()); index++)
	 writeImage(images, index, capPath.c_str(), cap->frameCounter());
   }
   return 0;
}
//cerr << i << " " << max_idx[0] << " " << max_idx[1] << " " <<max_idx[2] << " " << images[i].cols <<  " " << images[i].rows << " " << type2str(images[i].type()) << endl;
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
