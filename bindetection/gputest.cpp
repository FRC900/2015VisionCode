// WARNING: this sample is under construction! Use it on your own risk.
#include <iostream>
#include <fstream>
#include <cstdio>
#include "opencv2/gpu/gpu.hpp"

#include "classifierio.hpp"
#include "imagedetect.hpp"

using namespace std;
using namespace cv;
using namespace cv::gpu;

int main(int argc, char **argv)
{
   const string fname = "gputest.log";
   int dnum = 0;
   int snum = 0;

   {
      string str;
      string pf;
      int dir;
      int stage;
      ifstream ifile(fname.c_str());
      ofstream ofile((fname + ".new").c_str());
      if (ifile.is_open() && ofile.is_open())
      {
	 while (ifile >> dir >> stage >> str >> pf)
	 {
	    dnum = dir;
	    snum = stage;
	    ofile << dir << " " << stage << " " << str << " " << pf << endl;
	 }
      }
      snum += 1;
      if (snum > 100)
      {
	 dnum += 1;
	 snum = 0;
      }

      // If no status for last line, write it out now
      if (pf.length())
      {
	 ofile << dir << " " << stage <<" " << str << " Fail" << endl;
	 snum += 1;
	 if (snum > 100)
	 {
	    dnum += 1;
	    snum = 0;
	 }
      }

      if (ifile.is_open())
	 ifile.close();
      if (ofile.is_open())
	 ofile.close();
   }

   remove(fname.c_str());
   rename((fname + ".new").c_str(), fname.c_str());

   GPU_CascadeDetect *detectClassifier = NULL;
   Mat frame = imread("gasource/video.mp4_0001_0000_0329_0088_0290_0287.png");
   ofstream ofile(fname.c_str(), std::ofstream::app);

   for (; dnum < 25; dnum++)
   {
      for (; snum < 100; snum++)
      {
	 string name = getClassifierName(dnum, snum);
	 ofile << dnum << " " << snum << " " << name << flush;
	 ofile.flush();

	 // Delete the old  if it has been initialized
	 if (detectClassifier)
	    delete detectClassifier;
	 detectClassifier = new GPU_CascadeDetect(name.c_str());

	 // Verfiy the  loaded
	 if( !detectClassifier->loaded() )
	    ofile << " Fail" << endl << flush; 
	 else
	 {
	    vector<Rect> detectRects;
	    vector<unsigned> detectDirections;
	    detectClassifier->cascadeDetect(frame, detectRects, detectDirections); 
	    ofile << " Pass" << endl << flush;
	 }
      }
   }
}
