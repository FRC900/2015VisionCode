#ifndef VIDEOIN_HPP__
#define VIDEOIN_HPP__

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

class VideoIn
{
   public:
      Camera
      VideoIn();
      bool getFrame(bool pause=false,bool left=true, Mat &frame);

   private:
      cv::Mat          _frame;
};
#endif

