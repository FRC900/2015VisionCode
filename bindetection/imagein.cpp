#include "imagein.hpp"

using namespace cv;

ImageIn::ImageIn(const char *path)
{
   _frame = imread(path);
}

bool ImageIn::getNextFrame(Mat &frame, bool pause)
{
   frame = _frame.clone();
   return true;
}

int ImageIn::width(void)
{
   return _frame.cols;
}
int ImageIn::height(void)
{
   return _frame.rows;
}

