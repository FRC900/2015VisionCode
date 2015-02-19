#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <opencv2/opencv.hpp>

#include <iostream>
#include <stdio.h>

#include "imagedetect.hpp"
#include "videoin_c920.hpp"
#include "track.hpp"

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
   const string captureAllOpt = "--all";

   // Flags for various UI features
   bool pause = false;        // pause playback?
   bool captureAll = false;   // capture all found targets to image files?
   bool tracking = true;      // display tracking info?
   bool printFrames = false;  // print frame number?
   
   enum GPU_MODE
   {
      GPU_MODE_UNINITIALIZED,
      GPU_MODE_CPU,
      GPU_MODE_GPU
   };

   GPU_MODE gpuModeCurrent = GPU_MODE_UNINITIALIZED;
   GPU_MODE gpuModeNext    = GPU_MODE_CPU;
   if (gpu::getCudaEnabledDeviceCount() > 0)
      gpuModeNext = GPU_MODE_GPU;

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
	 else if (captureAllOpt.compare(0, captureAllOpt.length(), argv[fileArgc], captureAllOpt.length()) == 0)
	    captureAll = true;
	 else
	    break;
      }
      // Digit? Open camera
      if (isdigit(*argv[fileArgc]) && !strstr(argv[fileArgc],".jpg"))
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
	 // Open file name - will handle images or videos
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
   	
   
   string detectWindowName = "Detection Parameters";
   namedWindow(detectWindowName);
   createTrackbar ("Scale", detectWindowName, &scale, 50, NULL);
   createTrackbar ("Neighbors", detectWindowName, &neighbors, 50, NULL);
   createTrackbar ("Max Detect", detectWindowName, &maxDetectSize, 1000, NULL);
   createTrackbar ("GPU Scale", detectWindowName, &gpuDownScale, 20, NULL);
   const char *cascadeName = "../cascade_training/classifier_bin_6/cascade_oldformat_49.xml";
   // Use GPU code if hardware is detected, otherwise
   // fall back to CPU code
   BaseCascadeDetect *detectCascade = NULL;;
   
   if (!cap->getNextFrame(false, frame))
   {
      cerr << "Can not read frame from input" << endl;
      return 0;
   }
     
   // Minimum size of a bin at ~30 feet distance
   // TODO : Verify this once camera is calibrated
   minDetectSize = frame.cols * 0.04;

   // Create list of tracked objects
   TrackedObjectList binTrackingList(24.0, frame.cols);

#define frameTicksLength (sizeof(frameTicks) / sizeof(frameTicks[0]))
   double frameTicks[20];
   int64 startTick;
   int64 endTick;
   size_t frameTicksIndex = 0;
   // Read the video stream
   while(cap->getNextFrame(pause, frame))
   {
      startTick = getTickCount();
      //TODO : grab angle delta from robot
      // Adjust the position of all of the detected objects
      // to account for movement of the robot between frames
      double deltaAngle = 0.0;
      binTrackingList.adjustAngle(deltaAngle);

      if ((gpuModeCurrent == GPU_MODE_UNINITIALIZED) || (gpuModeCurrent != gpuModeNext))
      {
	 if (detectCascade)
	    delete detectCascade;
	 if (gpuModeNext == GPU_MODE_GPU)
	    detectCascade = new GPU_CascadeDetect(cascadeName);
	 else
	    detectCascade = new CPU_CascadeDetect(cascadeName);
	 gpuModeCurrent = gpuModeNext;

	 // Load the cascades
	 if( !detectCascade->loaded() )
	 {
	    cerr << "--(!)Error loading " << cascadeName << endl; 
	    return -1; 
	 }
      }
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
	 for (size_t j = 0; j < detectRects.size(); j++) {
	    if (i != j) {
	      Rect intersection = detectRects[i] & detectRects[j];
	      if (intersection.width * intersection.height > 0)
	      if (abs((detectRects[i].width * detectRects[i].height) - (detectRects[j].width * detectRects[j].height)) < 2000)
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
			//cout << "found intersection" << endl;
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
	 putText(frame, label.str(), Point(detectRects[i].x+10, detectRects[i].y+30), 
	       FONT_HERSHEY_PLAIN, 2.0, Scalar(0, 0, 255));

	 // Process this detected rectangle - either update the nearest
	 // object or add it as a new one
	 binTrackingList.processDetect(detectRects[i]);
      }
      // Print detect status of live objects
      binTrackingList.print();

      if (tracking)
      {
	 // Grab info from detected objects, print it ou
	 vector<TrackedObjectDisplay> displayList;
	 binTrackingList.getDisplay(displayList);
	 for (size_t i = 0; i < displayList.size(); i++)
	 {
	    if (displayList[i].ratio < 0.15)
	       continue;

	    // Color moves from red to green (via brown, yuck) 
	    // as the detected ratio goes up
	    Scalar rectColor(0, 255 * displayList[i].ratio, 255 * (1.0 - displayList[i].ratio));

	    // Highlight detected target
	    rectangle(frame, displayList[i].rect, rectColor, 3);

	    // Write detect ID, distance and angle data
	    putText(frame, displayList[i].id, Point(displayList[i].rect.x+25, displayList[i].rect.y+30), FONT_HERSHEY_PLAIN, 2.0, rectColor);
	    stringstream distLabel;
	    distLabel << "D=";
	    distLabel << displayList[i].distance;
	    putText(frame, distLabel.str(), Point(displayList[i].rect.x+10, displayList[i].rect.y+50), FONT_HERSHEY_PLAIN, 1.5, rectColor);
	    stringstream angleLabel;
	    angleLabel << "A=";
	    angleLabel << displayList[i].angle;
	    putText(frame, angleLabel.str(), Point(displayList[i].rect.x+10, displayList[i].rect.y+70), FONT_HERSHEY_PLAIN, 1.5, rectColor);
	 }
      }
      // Don't update to next frame if paused to prevent
      // objects missing from this frame to be aged out
      if (!pause)
	 binTrackingList.nextFrame();

      if (printFrames && cap->VideoCap())
      {
	 int frames = cap->VideoCap()->get(CV_CAP_PROP_FRAME_COUNT);
	 stringstream ss;
	 ss << cap->frameCounter();
	 ss << '/';
	 ss << frames;
	 putText(frame, ss.str(), Point(frame.cols - 15 * ss.str().length(), 20), FONT_HERSHEY_PLAIN, 1.5, Scalar(0,0,255));
      }
      if (frameTicksIndex >= frameTicksLength)
      {
	 double sum = 0;
	 for (size_t i = 0; i < frameTicksLength; i++)
	    sum += frameTicks[i];
	 sum /= frameTicksLength;
	 stringstream ss;
	 ss.precision(3);
	 ss << 1.0/sum;
	 ss << " FPS";
	 putText(frame, ss.str(), Point(frame.cols - 15 * ss.str().length(), 50), FONT_HERSHEY_PLAIN, 1.5, Scalar(0,0,255));

      }
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
      else if (c == 't') // toggle tracking info display
      {
	 tracking = !tracking;
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
      else if (c == 'P')
      {
	 printFrames = !printFrames;
      }
      else if (c == 'G') // toggle CPU/GPU mode
      {
	 if (gpuModeNext == GPU_MODE_GPU)
	    gpuModeNext = GPU_MODE_CPU;
	 else
	    gpuModeNext = GPU_MODE_GPU;
      }
      else if (isdigit(c)) // save a single detected image
      {
	 Mat frameCopy;
	 cap->getNextFrame(true, frameCopy);
	 writeImage(frameCopy, detectRects, c - '0', capPath.c_str(), cap->frameCounter());
      }
      if (captureAll && detectRects.size())
      {
	 Mat frameCopy;
	 cap->getNextFrame(true, frameCopy);
	 for (size_t index = 0; index < detectRects.size(); index++)
	    writeImage(frameCopy, detectRects, index, capPath.c_str(), cap->frameCounter());
      }
      endTick = getTickCount();
      frameTicks[frameTicksIndex++ % frameTicksLength] = (double)(endTick - startTick) / getTickFrequency();
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
