#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <opencv2/opencv.hpp>

#include <iostream>
#include <stdio.h>

#include "imagedetect.hpp"
#include "videoin_c920.hpp"

using namespace std;
using namespace cv;

void writeImage(const Mat &frame, const vector<Rect> &rects, size_t index, const char *path, int frameCounter);

int main( int argc, const char** argv )
{
   string windowName = "Capture - Face detection";
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
      capPath = "negative/2-11";
      windowName = "Camera 0";
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
	 capPath = "negative/2-11_" + (*argv[fileArgc] - '0');
	 stringstream ss;
	 ss << "Camera ";
	 ss << *argv[fileArgc] - '0';
	 windowName = ss.str();
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
	 windowName = argv[fileArgc];
      }
   }

   Mat frame;
   vector <Mat> images;
   	
   bool pause = false;
   bool captureAll = false;
   
   namedWindow("Parameters", WINDOW_AUTOSIZE);
   createTrackbar ("Scale", "Parameters", &scale, 50, NULL);
   createTrackbar ("Neighbors", "Parameters", &neighbors, 50, NULL);
   createTrackbar ("Max Detect", "Parameters", &maxDetectSize, 1000, NULL);

   const char *cascadeName = "../cascade_training/classifier_bin_6/cascade_oldformat_38.xml";
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
      // TODO : Verify this once camera is calibrated
      minDetectSize = frame.cols * 0.057;

      // Apply the classifier to the frame
      vector<Rect> detectRects;
      vector<unsigned> detectDirections;
      detectCascade->cascadeDetect(frame, detectRects, detectDirections); 

      images.clear();
      // Filter out images using threshold values - 
      // since bins are green this could be used as a second pass
      // to get rid of false positives which aren't green enough
      vector <Rect> filteredRects;

      for( size_t i = 0; i < min(detectRects.size(), detectMax); i++ ) 
      {
	 for (int j = 0; j < detectRects.size(); j++) {
	    if (i != j) {
	       Rect intersection = detectRects[i] & detectRects[j];
	       if (intersection.width * intersection.height > 0)
		  if (intersection.width / intersection.height < 5 &&  intersection.width / intersection.height > 0) {
		     Rect lowestYVal;
		     int indexHighest;
		     if(detectRects[i].y < detectRects[j].y) {
			lowestYVal = detectRects[i]; //higher rectangle
			indexHighest = j;
		     }
		     else {	
			lowestYVal = detectRects[j]; //higher rectangle
			indexHighest = i;
		     }
		     if(intersection.y > lowestYVal.y) {
			cout << "found intersection" << endl;
			rectangle(frame, detectRects[indexHighest], Scalar(0,255,255), 3);
			detectRects.erase(detectRects.begin()+indexHighest);
			detectDirections.erase(detectDirections.begin()+indexHighest);
		     }				
		  }
	    }
	 }
	 // Mark detected rectangle on image
	 Scalar rectColor;
	 switch (detectDirections[i])
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

	 rectangle( frame, detectRects[i], rectColor, 3);
	 // Label each outlined image with a digit.  Top-level code allows
	 // users to save these small images by hitting the key they're labeled with
	 // This should be a quick way to grab lots of falsly detected images
	 // which need to be added to the negative list for the next
	 // pass of classifier training.
	 stringstream label;
	 label << i;
	 putText(frame, label.str(), Point(detectRects[i].x+15, detectRects[i].y+15), 
	       FONT_HERSHEY_PLAIN, 2.0, Scalar(0, 0, 255));

	 // Code to determine image distance and angle offset
	 const double FOV = 70.42; // horizontal field of view of C920 camera
	 float FOVFrac = (float)(detectRects[i].width) / (float)frame.cols;
	 float totalFOV = 12.0 / FOVFrac;
	 float FOVRad =  (M_PI / 180.0) * (FOV / 2.0);
	 cout << frame.cols << " " << FOVFrac << " " << totalFOV << " " << FOVRad << endl;
	 distanceVal = totalFOV / tan(FOVRad);

	 float degreesPerPixel = FOV / frame.cols;
	 int rectCenterX = detectRects[i].x + (detectRects[i].width / 2);
	 int rectLocX = rectCenterX - (frame.cols / 2);
	 float degreesToTurn = (float)rectLocX * degreesPerPixel;
	 
	 double radiansToTurn = (M_PI / 180.0) * degreesToTurn;
	 double distanceToTurn = distanceVal * tan(radiansToTurn);
	 double slantDistance = sqrt(distanceVal * distanceVal + distanceToTurn * distanceToTurn);

	 double halfImageWidthInDegrees = degreesPerPixel * (detectRects[i].width / 2.0);
	 double halfImageWidthInRadians = halfImageWidthInDegrees * (M_PI / 180.0);
	 cout << degreesPerPixel << " " << detectRects[i].x << " " << detectRects[i].width << " " << halfImageWidthInDegrees << " " << halfImageWidthInRadians << endl;
	 double distanceValKJ = 12.0 / tan(halfImageWidthInRadians);

	 cout << "distance : " << distanceVal << " " << slantDistance << " " << distanceValKJ << " turn: " << degreesToTurn << endl;
        
      }
      #if 0
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
      #endif

      // Put an A on the screen if capture-all is enabled so
      // users can keep track of that toggle's mode
      if (captureAll)
	 putText(frame, "A", Point(25,25), FONT_HERSHEY_PLAIN, 2.5, Scalar(0, 255, 255));

      //-- Show what you got
      imshow( windowName, frame );

      // Process user IO
      char c = waitKey(5);
      if( c == 'c' ) { break; } // exit
      else if( c == 'q' ) { break; } // exit
      else if( c == 27 ) { break; } // exit
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
	 Mat frameCopy;
	 cap->getNextFrame(true, frameCopy);
	 for (size_t index = 0; index < detectRects.size(); index++)
	    writeImage(frameCopy, detectRects, index, capPath.c_str(), cap->frameCounter());
      }
      else if (c == 'p') // print frame number to screen
      {
	 cout << cap->frameCounter() << endl;
      }
      else if (isdigit(c)) // save a single detected image
      {
	 Mat frameCopy;
	 cap->getNextFrame(true, frameCopy);
	 writeImage(frameCopy, detectRects, c - '0', capPath.c_str(), cap->frameCounter());
      }
      if (captureAll)
      {
	 Mat frameCopy;
	 cap->getNextFrame(true, frameCopy);
	 for (size_t index = 0; index < detectRects.size(); index++)
	    writeImage(frameCopy, detectRects, index, capPath.c_str(), cap->frameCounter());
      }
   }
   return 0;
}

void writeImage(const Mat &frame, const vector<Rect> &rects, size_t index, const char *path, int frameCounter)
{
   if (index < rects.size())
   {
      Mat image = frame(rects[index]);
      // Create filename, save image
      stringstream fn;
      fn << path;
      fn << "_";
      fn << frameCounter;
      fn << "_";
      fn << index;
      imwrite(fn.str().substr(fn.str().rfind('\\')+1) + ".png", image);

      // Save grayscale equalized version
      Mat frameGray;
      cvtColor( image, frameGray, CV_BGR2GRAY );
      equalizeHist( frameGray, frameGray );
      imwrite(fn.str().substr(fn.str().rfind('\\')+1) + "_g.png", frameGray);

      // Save 20x20 version of the same image
      Mat smallImg;
      resize(image, smallImg, Size(20,20));
      imwrite(fn.str().substr(fn.str().rfind('\\')+1) + "_s.png", smallImg);

      // Save grayscale equalized version of small image
      cvtColor( smallImg, frameGray, CV_BGR2GRAY );
      equalizeHist( frameGray, frameGray );
      imwrite(fn.str().substr(fn.str().rfind('\\')+1) + "_g_s.png", frameGray);
   }
}
