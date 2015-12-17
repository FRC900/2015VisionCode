#include "opencv2/imgproc/imgproc.hpp"

#include "camerain.hpp"

using namespace cv;

CameraIn::CameraIn(int stream, bool gui)
{
   _cap = VideoCapture(stream);
   _cap.set(CV_CAP_PROP_FPS, 30.0);
   _cap.set(CV_CAP_PROP_FRAME_WIDTH, 800);
   _cap.set(CV_CAP_PROP_FRAME_HEIGHT, 600);
   _frameCounter = 0;
}

bool CameraIn::getNextFrame(Mat &frame, bool pause)
{
   if (!pause)
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

int CameraIn::height(void)
{
   return _cap.get(CV_CAP_PROP_FRAME_HEIGHT);
}

int CameraIn::width(void)
{
   return _cap.get(CV_CAP_PROP_FRAME_WIDTH);
}

int CameraIn::frameCounter(void)
{
   return _frameCounter;
}
