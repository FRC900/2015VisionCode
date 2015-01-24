#ifndef VIDEOIN_HPP__
#define VIDEOIN_HPP__

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

class VideoIn
{
   public:
      VideoIn(const char *path);
      VideoIn(int stream = -1);

      bool getNextFrame(bool pause, cv::Mat &frame);
      int frameCounter(void);

   private:
      cv::VideoCapture _cap;
      cv::Mat          _frame;
      int              _frameCounter;
};
#endif

