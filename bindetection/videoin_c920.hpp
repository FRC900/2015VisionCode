#ifndef VIDEOIN_C920_HPP__
#define VIDEOIN_C920_HPP__

// video 4 linux code doesn't work on cygwin,
// so fall back to normal OpenCV videocapture code
#ifndef __linux
#include "videoin.hpp"
#else

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "../C920VideoCap/C920Camera.h"

class VideoIn
{
   public:
      VideoIn(const char *path);
      VideoIn(int stream = -1);

      cv::VideoCapture *VideoCap(void) 
      {
	 if (_video && !_c920)
	    return &_cap;
	 return NULL;
      }
      bool getNextFrame(bool pause, cv::Mat &frame);
      int frameCounter(void);

   private:
      v4l2::C920Camera _camera;
      cv::VideoCapture _cap;
      cv::Mat          _frame;
      int              _frameCounter;
      bool             _c920;
      bool             _video;
      int              _brightness;
      int              _contrast;
      int              _saturation;
      int              _sharpness;
      int              _gain;
      int              _focus;
      int              _backlightCompensation;
      int              _whiteBalanceTemperature;
};
#endif
#endif

