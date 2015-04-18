#ifndef INC_FRAMETICKER_HPP__
#define INC_FRAMETICKER_HPP__

#include <opencv2/opencv.hpp>
class FrameTicker
{
   public :
	  FrameTicker()
	  {
		 _start       = 0;
		 _Index       = 0;
		 _Length      = 6;
		 _frameTicks  = new double[_Length];
	  }
	  void start(void)
	  {
		 _start = cv::getTickCount();
	  }

	  void end(void)
	  {
		 int64 end = cv::getTickCount();
		 _frameTicks[_Index++ % _Length] = (double)(end - _start) / cv::getTickFrequency();
		 _start = 0;
	  }

	  bool valid(void) const
	  {
		 return _Index >= _Length;
	  }

	  double getFPS(void) const
	  {
		 double sum = 0.0;
		 for (size_t i = 0; i < _Length; i++)
			sum += _frameTicks[i];
		 return _Length / sum;
	  }
   private :
	  double *_frameTicks;
	  size_t  _Length;
	  size_t  _Index;
	  int64   _start;
};


#endif

