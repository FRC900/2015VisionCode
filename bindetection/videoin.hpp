#ifndef VIDEOIN_HPP__
#define VIDEOIN_HPP__

#include "opencv2/highgui/highgui.hpp"
#include "mediain.hpp"

class VideoIn : public MediaIn
{
   public:
      VideoIn(const char *path);
      bool getNextFrame(cv::Mat &frame, bool pause = false);
      int width();
      int height();
      int frameCount(void);
      int frameCounter(void);
      void frameCounter(int frameCount);

   private:
      cv::VideoCapture _cap;
      cv::Mat          _frame;
};
#endif

