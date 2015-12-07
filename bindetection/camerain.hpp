#ifndef CAMERAIN_HPP__
#define CAMERAIN_HPP__

#include "mediain.hpp"
#include "opencv2/highgui/highgui.hpp"

class CameraIn : public MediaIn
{
   public:
      CameraIn(int stream = -1, bool gui = false);
      bool getNextFrame(cv::Mat &frame, bool pause = false);

      int width(void);
      int height(void);
      int frameCounter(void);

   protected:
      int _frameCounter;
   private:
      cv::VideoCapture _cap;
      cv::Mat          _frame;
};
#endif

