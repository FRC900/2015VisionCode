#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <opencv2/opencv.hpp>

#include <iostream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include "networktables/NetworkTable.h"
#include "networktables2/type/NumberArray.h"

#include "classifierio.hpp"
#include "imagedetect.hpp"
#include "videoin_c920.hpp"
#include "track.hpp"


using namespace std;
using namespace cv;

void writeImage(const Mat &frame, const vector<Rect> &rects, size_t index, const char *path, int frameCounter);
string getDateTimeString(void);

void writeNetTableNumber(NetworkTable *netTable, string label, int index, double value);

// Allow switching between CPU and GPU for testing 
enum CLASSIFIER_MODE
{
   CLASSIFIER_MODE_UNINITIALIZED,
   CLASSIFIER_MODE_RELOAD,
   CLASSIFIER_MODE_CPU,
   CLASSIFIER_MODE_GPU
};
bool maybeReloadClassifier(BaseCascadeDetect *&detectClassifier, CLASSIFIER_MODE &modeCurrent, CLASSIFIER_MODE &modeNext, int DirNumber, int StageNumber);

struct Args
{
   bool captureAll;  // capture all found targets to image files?
   bool tracking;    // display tracking info?
   bool batchMode;   // non-interactive mode - no display, run through
                     // as quickly as possible. Combine with --all
   bool ds;          // driver-station?
   bool writeVideo;  // write captured video to output
   int  frameStart;  // frame number to start from
   string inputName; // input file name or camera number
};

bool processArgs(int argc, const char **argv, Args &args);
void openVideoCap(const string &fileName, VideoIn *&cap, string &capPath, string &windowName, bool gui);
string getVideoOutName(void);

int main( int argc, const char** argv )
{
   const size_t detectMax = 10;

   // Flags for various UI features
   bool pause = false;       // pause playback?
   bool printFrames = false; // print frame number?

   int frameDisplayFrequency = 1;
   
   CLASSIFIER_MODE classifierModeCurrent = CLASSIFIER_MODE_UNINITIALIZED;
   CLASSIFIER_MODE classifierModeNext    = CLASSIFIER_MODE_CPU;
   if (gpu::getCudaEnabledDeviceCount() > 0)
      classifierModeNext = CLASSIFIER_MODE_GPU;

   // Classifier directory and stage to start with
   int classifierDirNum   = 13;
   int classifierStageNum = 28;

   // Pointer to either CPU or GPU classifier
   BaseCascadeDetect *detectClassifier = NULL;;
   
   // Read through command line args, extract
   // cmd line parameters and input filename
   Args args;
   if (!processArgs(argc, argv, args))
      return -2;

   string windowName = "Bin detection"; // GUI window name
   string capPath; // Output directory for captured images
   VideoIn *cap; // video input - image, video or camera
   openVideoCap(args.inputName, cap, capPath, windowName, !args.batchMode);

   if (!args.batchMode)
      namedWindow(windowName, WINDOW_AUTOSIZE);

   // Seek to start frame if necessary
   if (args.frameStart > 0)
      cap->frameCounter(args.frameStart);

   Mat frame;

   // If UI is up, pop up the parameters window
   if (!args.batchMode)
   {
      string detectWindowName = "Detection Parameters";
      namedWindow(detectWindowName);
      createTrackbar ("Scale", detectWindowName, &scale, 50, NULL);
      createTrackbar ("Neighbors", detectWindowName, &neighbors, 50, NULL);
      createTrackbar ("Max Detect", detectWindowName, &maxDetectSize, 1000, NULL);
      createTrackbar ("GPU Scale", detectWindowName, &gpuScale, 100, NULL);
   }
   // Grab initial frame to figure out image size and so on
   if (!cap->getNextFrame(false, frame))
   {
      cerr << "Can not read frame from input" << endl;
      return 0;
   }
     
   // Minimum size of a bin at ~30 feet distance
   // TODO : Verify this once camera is calibrated
   minDetectSize = frame.cols * 0.06;
   cout << minDetectSize << endl;

   // Create list of tracked objects
   // recycling bins are 24" wide
   TrackedObjectList binTrackingList(24.0, frame.cols);

   NetworkTable::SetClientMode();
   NetworkTable::SetIPAddress("10.9.0.2"); 
   NetworkTable *netTable = NetworkTable::GetTable("VisionTable");
   const size_t netTableArraySize = 7; // 7 bins?
   NumberArray netTableArray;

   // 7 bins max, 3 entries each (confidence, distance, angle)
   netTableArray.setSize(netTableArraySize * 3);

   // Code to write video frames to avi file on disk
   const int fourCC = CV_FOURCC('M','J','P','G');

   string videoOutName = getVideoOutName();
   Size S(frame.cols, frame.rows);
   VideoWriter outputVideo;
   args.writeVideo = netTable->GetBoolean("WriteVideo", args.writeVideo);
   const int videoWritePollFrequency = 60; // check for network table entry every this many frames (~5 seconds or so)
   int videoWritePollCount = videoWritePollFrequency;

   // Frame timing information
#define frameTicksLength (sizeof(frameTicks) / sizeof(frameTicks[0]))
   double frameTicks[3];
   int64 startTick;
   int64 endTick;
   size_t frameTicksIndex = 0;

   // Start of the main loop
   //  -- grab a frame
   //  -- update the angle of tracked objects 
   //  -- do a cascade detect on the current frame
   //  -- add those newly detected objects to the list of tracked objects
   while(cap->getNextFrame(pause, frame))
   {
      startTick = getTickCount(); // start time for this frame

      if (--videoWritePollCount == 0)
      {
	 args.writeVideo = netTable->GetBoolean("WriteVideo", args.writeVideo);
	 videoWritePollCount = videoWritePollFrequency;
      }
      if (args.writeVideo) {
	 if (!outputVideo.isOpened())
	    outputVideo.open(videoOutName.c_str(), CV_FOURCC('M','J','P','G'), 15, S, true);
	Mat writeCopy;
	writeCopy = frame.clone();
	 time_t rawTime;
	 time(&rawTime);
	 struct tm * localTime;
	 localTime = localtime(&rawTime);
	 char arrTime[100];
	 strftime(arrTime, sizeof(arrTime), "%T %D", localTime);
	 putText(writeCopy,string(arrTime), Point(0,20), FONT_HERSHEY_TRIPLEX, 0.75, Scalar(147,20,255), 1);
	 string matchNum = netTable->GetString("Match Number", "No Match Number");
	 if (matchNum != "No Match Number")
	 	matchNum = "Match Number:" + matchNum;
	 putText(writeCopy,matchNum,Point(0,40), FONT_HERSHEY_TRIPLEX, 0.75, Scalar(147,20,255), 1);
	 outputVideo << writeCopy;
      }
      //TODO : grab angle delta from robot
      // Adjust the position of all of the detected objects
      // to account for movement of the robot between frames
      double deltaAngle = 0.0;
      binTrackingList.adjustAngle(deltaAngle);

      if (!maybeReloadClassifier(detectClassifier, classifierModeCurrent, classifierModeNext, classifierDirNum, classifierStageNum))
	 return -1;

      // Apply the classifier to the frame
      // detectRects is a vector of rectangles, one for each detected object
      // detectDirections is the direction of each detected object - we might not use this
      vector<Rect> detectRects;
      vector<unsigned> detectDirections;
      detectClassifier->cascadeDetect(frame, detectRects, detectDirections); 

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
			if (!args.batchMode && ((cap->frameCounter() % frameDisplayFrequency) == 0))
			   rectangle(frame, detectRects[indexHighest], Scalar(0,255,255), 3);
			detectRects.erase(detectRects.begin()+indexHighest);
			detectDirections.erase(detectDirections.begin()+indexHighest);
		     }				
		  }
	    }
	 }
	 if (!args.batchMode && ((cap->frameCounter() % frameDisplayFrequency) == 0))
	 {
	    // Mark detected rectangle on image
	    // Change color based on direction we think the bin is pointing
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
	 }

	 // Process this detected rectangle - either update the nearest
	 // object or add it as a new one
	 binTrackingList.processDetect(detectRects[i]);
      }

#if 0
      // Print detect status of live objects
      if (args.tracking)
	 binTrackingList.print();
#endif
      // Grab info from trackedobjects, print it out
      vector<TrackedObjectDisplay> displayList;
      binTrackingList.getDisplay(displayList);

      // Clear out network table array
      for (size_t i = 0; !args.ds & (i < (netTableArraySize * 3)); i++)
	 netTableArray.set(i, -1);
      for (size_t i = 0; !args.ds & (i < netTableArraySize); i++)
      {
	 writeNetTableNumber(netTable,"Ratio", i, -1);
	 writeNetTableNumber(netTable,"Distance", i, -1);
	 writeNetTableNumber(netTable,"Angle", i, -1);
      }

      for (size_t i = 0; i < displayList.size(); i++)
      {
	 if ((displayList[i].ratio >= 0.15) && args.tracking && 
	    !args.batchMode && ((cap->frameCounter() % frameDisplayFrequency) == 0))
	 {
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

	 if (!args.ds && (i < netTableArraySize))
	 {
	    netTableArray.set(i*3,   displayList[i].ratio);
	    netTableArray.set(i*3+1, displayList[i].distance);
	    netTableArray.set(i*3+2, displayList[i].angle);
	    writeNetTableNumber(netTable,"Ratio", i, displayList[i].ratio);
	    writeNetTableNumber(netTable,"Distance", i, displayList[i].distance);
	    writeNetTableNumber(netTable,"Angle", i, displayList[i].angle);
#if 0
	    cout << i << " ";
	    cout << displayList[i].ratio << " ";
	    cout << displayList[i].distance << " ";
	    cout << displayList[i].angle << endl;
#endif
	 }
      }

      if (!args.ds)
      {
	 netTable->PutValue("VisionArray", netTableArray);
	 netTable->PutNumber("FrameNumber", cap->frameCounter());
      }

      // Don't update to next frame if paused to prevent
      // objects missing from this frame to be aged out
      // as the current frame is redisplayed over and over
      if (!pause)
	 binTrackingList.nextFrame();

      // Print frame number of video if the option is enabled
      if (!args.batchMode && ((cap->frameCounter() % frameDisplayFrequency) == 0)&& printFrames && cap->VideoCap())
      {
	 stringstream ss;
	 ss << cap->frameCounter();
	 ss << '/';
	 ss << cap->VideoCap()->get(CV_CAP_PROP_FRAME_COUNT);
	 putText(frame, ss.str(), Point(frame.cols - 15 * ss.str().length(), 20), FONT_HERSHEY_PLAIN, 1.5, Scalar(0,0,255));
      }

      // Display current classifier under test
      {
	 stringstream ss;
	 ss << classifierDirNum;
	 ss << ',';
	 ss << classifierStageNum;
	 putText(frame, ss.str(), Point(0, frame.rows- 30), FONT_HERSHEY_PLAIN, 1.5, Scalar(0,0,255));
      }

      // For interactive mode, update the FPS as soon as we have
      // a complete array of frame time entries
      // For args.batch mode, only update every frameTicksLength frames to
      // avoid printing too much stuff
      if ((!args.batchMode && ((cap->frameCounter() % frameDisplayFrequency) == 0) && (frameTicksIndex >= frameTicksLength)) ||
	  (args.batchMode && ((frameTicksIndex % (frameTicksLength*10)) == 0)))
      {
	 // Get the average frame time over the last
	 // frameTicksLength frames
	 double sum = 0.0;
	 for (size_t i = 0; i < frameTicksLength; i++)
	    sum += frameTicks[i];
	 sum /= frameTicksLength;
	 stringstream ss;
	 // If in args.batch mode and reading a video, display
	 // the frame count
	 if (args.batchMode && cap->VideoCap())
	 {
	    ss << cap->frameCounter();
	    ss << '/';
	    ss << cap->VideoCap()->get(CV_CAP_PROP_FRAME_COUNT);
	    ss << " : ";
	 }
	 // Print the FPS
	 ss.precision(3);
	 ss << 1.0 / sum;
	 ss << " FPS";
	 if (!args.batchMode)
	    putText(frame, ss.str(), Point(frame.cols - 15 * ss.str().length(), 50), FONT_HERSHEY_PLAIN, 1.5, Scalar(0,0,255));
	 else
	    cout << ss.str() << endl;
      }
      // Driverstation Code
      if (args.ds)
      {
	 // Report boolean value for each bin on the step
	 bool hits[4];
	 for (int i = 0; i < 4; i++)
	 {
	    Rect dsRect(i * frame.cols / 4, 0, frame.cols/4, frame.rows);
	    if (!args.batchMode && ((cap->frameCounter() % frameDisplayFrequency) == 0))
	       rectangle(frame, dsRect, Scalar(0,255,255,3));
	    hits[i] = false;
	    // For each quadrant of the field, look for a detected
	    // rectangle contained entirely in the quadrant
	    // Assume that if that's found, it is a bin
	    // TODO : Tune this later with a distance range
	    for( size_t j = 0; j < displayList.size(); j++ ) 
	    {
	       if (((displayList[j].rect & dsRect) == displayList[j].rect) && (displayList[j].ratio > 0.15))
	       {
		  if (!args.batchMode && ((cap->frameCounter() % frameDisplayFrequency) == 0))
			rectangle(frame, displayList[j].rect, Scalar(255,128,128), 3);
		  hits[i] = true;
	       }
	    }
	    stringstream ss;
	    ss << "Bin";
	    ss << (i+1);
	    netTable->PutBoolean(ss.str().c_str(), hits[i]);
	 }
      }

      if (!args.batchMode && ((cap->frameCounter() % frameDisplayFrequency) == 0))
      {
	 // Put an A on the screen if capture-all is enabled so
	 // users can keep track of that toggle's mode
	 if (args.captureAll)
	    putText(frame, "A", Point(25,25), FONT_HERSHEY_PLAIN, 2.5, Scalar(0, 255, 255));

	 //-- Show what you got
	 imshow( windowName, frame );

	 // Process user IO
	 char c = waitKey(5);
	 if ((c == 'c') || (c == 'q') || (c == 27)) 
	 { // exit
	    if (netTable->IsConnected())
	       NetworkTable::Shutdown();
	    break; 
	 } 
	 else if( c == ' ') { pause = !pause; }
	 else if( c == 'f')  // advance to next frame
	 {
	    cap->getNextFrame(false, frame);
	 }
	 else if (c == 'A') // toggle capture-all
	 {
	    args.captureAll = !args.captureAll;
	 }
	 else if (c == 't') // toggle args.tracking info display
	 {
	    args.tracking = !args.tracking;
	 }
	 else if (c == 'a') // save all detected images
	 {
	    // Save from a copy rather than the original
	    // so all the markup isn't saved, only the raw image
	    Mat frameCopy;
	    cap->getNextFrame(true, frameCopy);
	    for (size_t index = 0; index < detectRects.size(); index++)
	       writeImage(frameCopy, detectRects, index, capPath.c_str(), cap->frameCounter());
	 }
	 else if (c == 'p') // print frame number to console
	 {
	    cout << cap->frameCounter() << endl;
	 }
	 else if (c == 'P') // Toggle frame # printing to 
	 {
	    printFrames = !printFrames;
	 }
	 else if (c == 'S')
	 {
	    frameDisplayFrequency += 1;
	 }
	 else if (c == 's')
	 {
	    frameDisplayFrequency = max(1, frameDisplayFrequency - 1);
	 }
	 else if (c == 'G') // toggle CPU/GPU mode
	 {
	    if (classifierModeNext == CLASSIFIER_MODE_GPU)
	       classifierModeNext = CLASSIFIER_MODE_CPU;
	    else
	       classifierModeNext = CLASSIFIER_MODE_GPU;
	 }
	 else if (c == '.') // higher classifier stage
	 {
	    if (findNextClassifierStage(classifierDirNum, classifierStageNum, true))
	       classifierModeNext = CLASSIFIER_MODE_RELOAD;
	 }
	 else if (c == ',') // lower classifier stage
	 {
	    if (findNextClassifierStage(classifierDirNum, classifierStageNum, false))
	       classifierModeNext = CLASSIFIER_MODE_RELOAD;
	 }
	 else if (c == '>') // higher classifier dir num
	 {
	    if (findNextClassifierDir(classifierDirNum, classifierStageNum, true))
	       classifierModeNext = CLASSIFIER_MODE_RELOAD;
	 }
	 else if (c == '<') // higher classifier dir num
	 {
	    if (findNextClassifierDir(classifierDirNum, classifierStageNum, false))
	       classifierModeNext = CLASSIFIER_MODE_RELOAD;
	 }
	 else if (isdigit(c)) // save a single detected image
	 {
	    Mat frameCopy;
	    cap->getNextFrame(true, frameCopy);
	    writeImage(frameCopy, detectRects, c - '0', capPath.c_str(), cap->frameCounter());
	 }
      }
      // If args.captureAll is enabled, write each detected rectangle
      // to their own output image file
      if (args.captureAll && detectRects.size())
      {
	 // Save from a copy rather than the original
	 // so all the markup isn't saved, only the raw image
	 Mat frameCopy;
	 cap->getNextFrame(true, frameCopy);
	 for (size_t index = 0; index < min(detectRects.size(), detectMax); index++)
	    writeImage(frameCopy, detectRects, index, capPath.c_str(), cap->frameCounter());
      }
      // Save frame time for the current frame
      endTick = getTickCount();
      frameTicks[frameTicksIndex++ % frameTicksLength] = (double)(endTick - startTick) / getTickFrequency();
   }
   return 0;
}

// Write out the selected rectangle from the input frame
// Save multiple copies - the full size image, that full size image converted to grayscale and histogram equalized, and a small version of each.
// The small version is saved because while the input images to the training process are 20x20
// the detection code can find larger versions of them. Scale them down to 20x20 so the complete detected
// image is used as a negative to the training code. Without this, the training code will pull a 20x20
// sub-image out of the larger full image
void writeImage(const Mat &frame, const vector<Rect> &rects, size_t index, const char *path, int frameCounter)
{
   if (index < rects.size())
   {
      Mat image = frame(rects[index]);
      // Create filename, save image
      stringstream fn;
      fn << "negative/";
      fn << path;
      fn << "_";
      fn << frameCounter;
      fn << "_";
      fn << index;
      imwrite(fn.str() + ".png", image);

      // Save grayscale equalized version
      Mat frameGray;
      cvtColor( image, frameGray, CV_BGR2GRAY );
      equalizeHist( frameGray, frameGray );
      imwrite(fn.str() + "_g.png", frameGray);

      // Save 20x20 version of the same image
      Mat smallImg;
      resize(image, smallImg, Size(20,20));
      imwrite(fn.str() + "_s.png", smallImg);

      // Save grayscale equalized version of small image
      cvtColor( smallImg, frameGray, CV_BGR2GRAY );
      equalizeHist( frameGray, frameGray );
      imwrite(fn.str() + "_g_s.png", frameGray);
   }
}

string getDateTimeString(void)
{
   time_t rawtime;
   struct tm * timeinfo;

   time (&rawtime);
   timeinfo = localtime (&rawtime);

   stringstream ss;
   ss << timeinfo->tm_mon + 1;
   ss << "-";
   ss << timeinfo->tm_mday;
   ss << "_";
   ss << timeinfo->tm_hour;
   ss << "_";
   ss << timeinfo->tm_min;
   return ss.str();
}

// Process command line args. 
bool processArgs(int argc, const char **argv, Args &args)
{
   const string frameOpt      = "--frame=";
   const string captureAllOpt = "--all";
   const string batchModeOpt  = "--batch";
   const string dsOpt         = "--ds";
   const string writeVideoOpt = "--capture";
   const string badOpt        = "--";

   args.captureAll   = false;
   args.tracking     = true;
   args.batchMode    = false;
   args.ds           = false;
   args.writeVideo   = false;
   args.frameStart   = 0.0;
   args.inputName.clear();

   // Read through command line args, extract
   // cmd line parameters and input filename
   int fileArgc;
   for (fileArgc = 1; fileArgc < argc; fileArgc++)
   {
      if (frameOpt.compare(0, frameOpt.length(), argv[fileArgc], frameOpt.length()) == 0)
	 args.frameStart = atoi(argv[fileArgc] + frameOpt.length());
      else if (captureAllOpt.compare(0, captureAllOpt.length(), argv[fileArgc], captureAllOpt.length()) == 0)
	 args.captureAll = true;
      else if (batchModeOpt.compare(0, batchModeOpt.length(), argv[fileArgc], batchModeOpt.length()) == 0)
	 args.batchMode = true;
      else if (dsOpt.compare(0, dsOpt.length(), argv[fileArgc], dsOpt.length()) == 0)
	 args.ds = true;
      else if (writeVideoOpt.compare(0, writeVideoOpt.length(), argv[fileArgc], writeVideoOpt.length()) == 0)
	 args.writeVideo = true;
      else if (badOpt.compare(0, badOpt.length(), argv[fileArgc], badOpt.length()) == 0)
      {
	 cerr << "Unknown command line option " << argv[fileArgc] << endl;
	 return false; // unknown option
      }
      else // first non -- arg is filename or camera number
	 break;
   }

   if (fileArgc < argc)
      args.inputName = argv[fileArgc];
   return true;
}

// Open video capture object. Figure out if input is camera, video, image, etc
void openVideoCap(const string &fileName, VideoIn *&cap, string &capPath, string &windowName, bool gui)
{
   if (fileName.length() == 0)
   {
      // No arguments? Open default camera
      // and hope for the best
      cap        = new VideoIn(0, gui);
      capPath    = getDateTimeString();
      windowName = "Default Camera";
   }
   // Digit, but no dot (meaning no file extension)? Open camera
   // Also handle explicit -1 to pick the default camera
   else if ((fileName.find('.') == string::npos) &&
            (isdigit(fileName[0]) || fileName.compare("-1") == 0))
   {
      cap        = new VideoIn(fileName[0] - '0', gui);
      capPath    = getDateTimeString() + "_" + fileName;
      windowName = "Camera " + fileName;
   }
   else // has to be a file name, we hope
   {
      // Open file name - will handle images or videos
      cap = new VideoIn(fileName.c_str());

      // Strip off directory for capture path
      capPath = fileName;
      const size_t last_slash_idx = capPath.find_last_of("\\/");
      if (std::string::npos != last_slash_idx)
	 capPath.erase(0, last_slash_idx + 1);
      windowName = fileName;
   }
}

void writeNetTableNumber(NetworkTable *netTable, string label, int index, double value)
{
   stringstream ss;
   ss << label;
   ss << (index+1);
   netTable->PutNumber(ss.str().c_str(), value);
}

// Code to allow switching between CPU and GPU for testing
// Also used to reload different classifer stages on the fly
bool maybeReloadClassifier(BaseCascadeDetect *&detectClassifier, 
      CLASSIFIER_MODE &modeCurrent, 
      CLASSIFIER_MODE &modeNext, 
      int dirNum, int stageNum)
{
   if ((modeCurrent == CLASSIFIER_MODE_UNINITIALIZED) || 
       (modeCurrent != modeNext))
   {
      string name = getClassifierName(dirNum, stageNum);
cerr << name << endl;

      // If reloading with new name, keep the current
      // CPU/GPU mode setting 
      if (modeNext == CLASSIFIER_MODE_RELOAD)
	 modeNext = modeCurrent;

      // Delete the old  if it has been initialized
      if (detectClassifier)
	 delete detectClassifier;

      // Create a new CPU or GPU  based on the
      // user's selection
      if (modeNext == CLASSIFIER_MODE_GPU)
	 detectClassifier = new GPU_CascadeDetect(name.c_str());
      else
	 detectClassifier = new CPU_CascadeDetect(name.c_str());
      modeCurrent = modeNext;

      // Verfiy the  loaded
      if( !detectClassifier->loaded() )
      {
	 cerr << "--(!)Error loading " << name << endl; 
	 return false; 
      }
   }
   return true;
}

string getVideoOutName(void)
{
   int index = 0;
   int rc;
   struct stat statbuf;
   stringstream ss;
   do 
   {
      ss.str(string(""));
      ss.clear();
      ss << "cap";
      ss << index;
      ss << ".avi";
      rc = stat(ss.str().c_str(), &statbuf);
   }
   while (rc == 0);
   return ss.str();
}

