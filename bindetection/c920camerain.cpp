#include <iostream>
#include "c920camerain.hpp"
#ifdef _linux

using namespace cv;
using namespace std;

C920CameraIn::C920CameraIn(int _stream, bool gui)
{
   initCamera(_stream, gui);
}

C920CameraIn::initCamera(int _stream, bool gui)
{
   _frameCounter = 0;
   // Stream is a camera number, corresponding
   // to a given /dev/video?? device
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


   _captureSize = v4l2::CAPTURE_SIZE_640x480;
   _camera.ChangeCaptureSize(_captureSize);
   _camera.ChangeCaptureFPS(v4l2::CAPTURE_FPS_30);
   _camera.GetBrightness(_brightness);
   _camera.GetContrast(_contrast);
   _camera.GetSaturation(_saturation);
   _camera.GetSharpness(_sharpness);
   _camera.GetGain(_gain);
   _camera.GetBacklightCompensation(_backlightCompensation);
   _camera.GetWhiteBalanceTemperature(_whiteBalanceTemperature);
   ++_whiteBalanceTemperature;
  // _camera.GetFocus(_focus);
  // ++_focus;
   if (gui)
   {
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
}

bool C920CameraIn::getNextFrame(Mat &frame, bool pause)
{
   _camera.SetBrightness(_brightness);
   _camera.SetContrast(_contrast);
   _camera.SetSaturation(_saturation);
   _camera.SetSharpness(_sharpness);
   _camera.SetGain(_gain);
   _camera.SetBacklightCompensation(_backlightCompensation);
   _camera.SetWhiteBalanceTemperature(_whiteBalanceTemperature-1);
   _camera.SetFocus(_focus-1);
   if (!pause)
   {
	  if (_camera.GrabFrame())
		 _camera.RetrieveMat(_frame);
	  if( _frame.empty() )
		 return false;
	  if (_frame.rows > 800)
		 pyrDown(_frame, _frame);
	  _frameCounter += 1;
   }
   frame = _frame.clone();

   return true;
}

int C920CameraIn::width(void) const
{
   return v4l2::CAPTURE_SIZE_WIDTHS[_captureSize];
}

int C920CameraIn::height(void) const
{
   return v4l2::CAPTURE_SIZE_HEIGHTS[_captureSize];
}

#else

C920CameraIn::C920CameraIn(int _stream, bool gui)
{
}

bool C920CameraIn::getNextFrame(Mat &frame, bool pause)
{
   return false;
}

int C920CameraIn::width(void)
{
   return 0;
}

int C920CameraIn::height(void)
{
   return 0;
}

#endif
