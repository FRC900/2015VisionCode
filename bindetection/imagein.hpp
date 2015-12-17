#ifndef IMAGEIN_HPP__
#define IMAGEIN_HPP__

#include "opencv2/highgui/highgui.hpp"

#include "mediain.hpp"

class ImageIn : public MediaIn
{
   public:
      ImageIn(const char *path);
      bool getNextFrame(cv::Mat &frame, bool pause = false);

      int width();
      int height();

   private:
      cv::Mat _frame;
};
#endif

