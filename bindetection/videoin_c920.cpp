#ifndef __linux__
#include "videoin.cpp"
#else
#include "videoin_c920.hpp"

using namespace cv;
using namespace std;

VideoIn::VideoIn(const char *path)
{
   if (strstr(path, ".png") || strstr(path, ".jpg"))
   {
      _frame = imread(path);
      _video = false;
   }
   else
   {
      _cap = VideoCapture(path);
      _video = true;
   }
   _frameCounter = 0;
   _c920 = false;
}

VideoIn::VideoIn(int _stream)
{
   
   if (_stream < 0)
      _stream = 0;
   stringstream videoStream;

   videoStream << "/dev/video";
   videoStream << _stream;
   _camera.Open(videoStream.str().c_str());

   _brightness = 128;
   _contrast = 128;
   _saturation = 128;
   _sharpness = 128;
   _gain = 1;
   _focus = 1;
   _backlightCompensation = 0;
   _whiteBalanceTemperature = 0;
   _video        = true;
   _c920         = true;
   _frameCounter = 0;


   _camera.ChangeCaptureSize(v4l2::CAPTURE_SIZE_640x480);
   _camera.ChangeCaptureFPS(v4l2::CAPTURE_FPS_30);
   _camera.GetBrightness(_brightness);
   _camera.GetContrast(_contrast);
   _camera.GetSaturation(_saturation);
   _camera.GetSharpness(_sharpness);
   _camera.GetGain(_gain);
   _camera.GetBacklightCompensation(_backlightCompensation);
   _camera.GetWhiteBalanceTemperature(_whiteBalanceTemperature);
   ++_whiteBalanceTemperature;
   _camera.GetFocus(_focus);
   ++_focus;
   cv::namedWindow("Adjustments", CV_WINDOW_NORMAL);
   cv::createTrackbar("Brightness", "Adjustments", &_brightness, 255);
   cv::createTrackbar("Contrast", "Adjustments", &_contrast, 255);
   cv::createTrackbar("Saturation", "Adjustments", &_saturation, 255);
   cv::createTrackbar("Sharpness", "Adjustments", &_sharpness, 255);
   cv::createTrackbar("Gain", "Adjustments", &_gain, 255);
   cv::createTrackbar("Backlight Compensation", "Adjustments", &_backlightCompensation, 1);
   // Off by one to account for -1 being auto.
   cv::createTrackbar("White Balance Temperature", "Adjustments", &_whiteBalanceTemperature, 6501);
   cv::createTrackbar("Focus", "Adjustments", &_focus, 256);
}

bool VideoIn::getNextFrame(bool pause, Mat &frame)
{
   if (_c920)
   {
      _camera.SetBrightness(_brightness);
      _camera.SetContrast(_contrast);
      _camera.SetSaturation(_saturation);
      _camera.SetSharpness(_sharpness);
      _camera.SetGain(_gain);
      _camera.SetBacklightCompensation(_backlightCompensation);
      --_whiteBalanceTemperature;
      _camera.SetWhiteBalanceTemperature(_whiteBalanceTemperature);
      ++_whiteBalanceTemperature;
      --_focus;
      _camera.SetFocus(_focus);
      ++_focus;
   }
   if (!pause && _video)
   {
       if (_c920)
       {
	  if (_camera.GrabFrame())
	     _camera.RetrieveMat(_frame);
       }
       else
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
int VideoIn::frameCounter(void)
{
   return _frameCounter;
}
#endif
