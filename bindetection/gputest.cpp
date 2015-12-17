#include <iostream>
#include <fstream>
#include <cstdio>

#include <opencv2/highgui/highgui.hpp>

#include "classifierio.hpp"
#include "objdetect.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

int main(int argc, char **argv)
{
   const string fname = "gputest.log";
   int dnum = 1;
   int snum = 0;
   scale = 200;
   minDetectSize = 200;

   // Scan through gputest.log and grab previous results
   // Likely if a classifier stage fails the program will
   // crash so by reading the last thing written the code
   // can restart right after the failing stage
   {
      string str; // Classifier name
      string pf;  // pass or fail string
      int dir;    // dir number read from file
      int stage;  // stage # left from file
      bool foundInput = false; // is input log empty?
      ifstream ifile(fname.c_str());
      ofstream ofile((fname + ".new").c_str());
      if (ifile.is_open() && ofile.is_open())
      {
	 // Even a failing line will have a dir, stage, and name
	 while (ifile >> dir >> stage >> str)
	 {
	    dnum = dir;
	    snum = stage;
	    foundInput = true;
	    ofile << dir << " " << stage << " " << str << " " ;
	    cout<< "Read " << dir << " " << stage << " " << str << " ";

	    // Look for pass or fail
	    pf = string("");
	    if (ifile >> pf)
	    {
	       ofile << pf << endl << flush;
	       cout  << pf << endl << flush;
	    }
	    else // if not found, end of file. Restart at next stage
	       break;
	 }
      }
      // If no status for last line, write it out now so the file
      // format is correct
      if (foundInput && !pf.length())
      {
	 ofile << "Fail" << endl;
	 cout << "Fail" << endl;
      }
      cout << "Now testing from " << dnum << " " << snum << endl;

      if (ifile.is_open())
	 ifile.close();
      if (ofile.is_open())
	 ofile.close();
   }

   // Remove old log, replace it with log just created
   remove(fname.c_str());
   rename((fname + ".new").c_str(), fname.c_str());

   GPU_CascadeDetect *detector = NULL;
   Mat frame = imread("gaSource/video1.mp4_0001_0000_0427_0187_0165_0164.png");
   ofstream ofile(fname.c_str(), std::ofstream::app);

   // Scan through dirs and stages
   // For each one found, try to load it into the
   // GPU and see if the program fails
   for (; dnum < 25; dnum++)
   {
      while (snum < 100)
      {
	 ClassifierIO classifierIO("/home/ubuntu/2015VisionCode/cascade_training/classifier_bin_", dnum, snum);
	 if (classifierIO.findNextClassifierStage(true))
	 {
	    string name = classifierIO.getClassifierName();
	    cout << dnum << " " << snum << " " << name << flush;
	    ofile << dnum << " " << snum << " " << name << flush;

	    // Delete the old  if it has been initialized
	    if (detector)
	       delete detector;
	    detector = new GPU_CascadeDetect(name.c_str());

	    // Verfiy the detector loaded
	    if( !detector->initialized() )
	    {
	       ofile << " Fail" << endl << flush; 
	       cout  << " Fail" << endl << flush; 
	    }
	    else
	    {
	       vector<Rect> detectRects;
	       detector->Detect(frame, detectRects);
	       // Some detectors crash with certain classifier
	       // inputs.  If we blow up, the following lines
	       // won't be written to the output
	       ofile << " Pass" << endl << flush;
	       cout << " Pass" << endl << flush;
	    }
	 }
	 else break;
      }
      snum = 0;
   }
}
