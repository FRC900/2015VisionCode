#include "opencv2/imgproc/imgproc.hpp"

#include "videoin.hpp"

using namespace cv;
using namespace std;

VideoIn::VideoIn(const char *path)
{
   _cap = VideoCapture(path);
}

bool VideoIn::getNextFrame(Mat &frame, bool pause)
{
   if (!pause)
   {
      _cap >> _frame;
      if( _frame.empty() )
	 return false;
      if (_frame.rows > 800)
	 pyrDown(_frame, _frame);
   }
   frame = _frame.clone();

   return true;
}

int VideoIn::width()
{
   return _cap.get(CV_CAP_PROP_FRAME_WIDTH);
}

int VideoIn::height()
{
   return _cap.get(CV_CAP_PROP_FRAME_HEIGHT);
}

int VideoIn::frameCount(void)
{
   return _cap.get(CV_CAP_PROP_FRAME_COUNT);
}

int VideoIn::frameCounter(void)
{
   return _cap.get(CV_CAP_PROP_POS_FRAMES);
}

void VideoIn::frameCounter(int frameCount)
{
   _cap.set(CV_CAP_PROP_POS_FRAMES, frameCount);
}
