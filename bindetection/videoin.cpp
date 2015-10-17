#include "videoin.hpp"
#include <iostream>

using namespace cv;
using namespace std;

VideoIn::VideoIn(const char *path)
{
   if (strstr(path, ".png") || strstr(path, ".jpg"))
   {
      _frame = imread(path);
      _video = false;
   }
   else
   {
      //open video?
   }
   _frameCounter = 0;
}
VideoIn::VideoIn(int stream, bool gui)
{
   _cap = VideoCapture(stream);
   _video = true;
   _frameCounter = 0;
}

bool VideoIn::getNextFrame(bool pause, Mat &frame)
{
   if (!pause && _video)
   {
      _cap >> _frame;
      if( _frame.empty() )
	 return false;
      if (_frame.rows > 800)
	 pyrDown(_frame, _frame);
      _frameCounter += 1;
   }
   frame = _frame.clone();

   return true;
}
int VideoIn::frameCounter(void)
{
   return _frameCounter;
}

VideoCapture *VideoIn::VideoCap(void) 
{
   if (_video)
      return &_cap;
   return NULL;
}
void VideoIn::frameCounter(int frameCount)
{
   if (_video)
      _cap.set(CV_CAP_PROP_POS_FRAMES, frameCount);
   _frameCounter = frameCount;
}

