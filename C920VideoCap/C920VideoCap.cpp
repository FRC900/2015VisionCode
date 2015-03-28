/*
 * Main.cpp
 *
 * Created on: Dec 31, 2014
 * Author: jrparks
 */
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include "C920Camera.h"

using namespace std;
using namespace cv;

int Brightness = 128,
    Contrast = 128,
    Saturation = 128,
    Sharpness = 128,
    Gain = 1,
    Focus = 1,
    BacklightCompensation = 0,
    WhiteBalanceTemperature = 0;
v4l2::C920Camera camera;
cv::Mat frame;

int main(int argc, char* argv[]) {
   fprintf(stdout, "Preparing to open camera.\n");
   camera.Open("/dev/video0");
   if (!camera.IsOpen()) {
      fprintf(stderr, "Unable to open camera.\n");
      return -1;
   }
   bool batch = false;

   if ((argc > 1) && (strcmp(argv[1], "--batch") == 0))
      batch = true;

   char name[256];
   int index = 0;
   int rc;
   struct stat statbuf;
   do 
   {
      sprintf(name, "cap%d.avi", index++);
      rc = stat(name, &statbuf);
   }
   while (rc == 0);
   fprintf (stderr, "Writing to %s\n", name);

   camera.ChangeCaptureSize(v4l2::CAPTURE_SIZE_800x600);
   camera.ChangeCaptureFPS(v4l2::CAPTURE_FPS_30);
   camera.GetBrightness(Brightness);
   camera.GetContrast(Contrast);
   camera.GetSaturation(Saturation);
   camera.GetSharpness(Sharpness);
   camera.GetGain(Gain);
   camera.GetBacklightCompensation(BacklightCompensation);
   camera.GetWhiteBalanceTemperature(WhiteBalanceTemperature);
   ++WhiteBalanceTemperature;
#if 0
   camera.GetFocus(Focus);
   ++Focus;
#endif
   cv::namedWindow("Adjustments", CV_WINDOW_NORMAL);
   cv::createTrackbar("Brightness", "Adjustments", &Brightness, 255);
   cv::createTrackbar("Contrast", "Adjustments", &Contrast, 255);
   cv::createTrackbar("Saturation", "Adjustments", &Saturation, 255);
   cv::createTrackbar("Sharpness", "Adjustments", &Sharpness, 255);
   cv::createTrackbar("Gain", "Adjustments", &Gain, 255);
   cv::createTrackbar("Backlight Compensation", "Adjustments", &BacklightCompensation, 1);
   // Off by one to account for -1 being auto.
   cv::createTrackbar("White Balance Temperature", "Adjustments", &WhiteBalanceTemperature, 6501);
   cv::createTrackbar("Focus", "Adjustments", &Focus, 256);

   camera.GrabFrame();  
   camera.RetrieveMat(frame);
   VideoWriter outputVideo(name, CV_FOURCC('M','J','P','G'), 30, Size(frame.cols, frame.rows), true);

   int wait_key = 0;
   while (true) {
      camera.SetBrightness(Brightness);
      camera.SetContrast(Contrast);
      camera.SetSaturation(Saturation);
      camera.SetSharpness(Sharpness);
      camera.SetGain(Gain);
      camera.SetBacklightCompensation(BacklightCompensation);
      --WhiteBalanceTemperature;
      camera.SetWhiteBalanceTemperature(WhiteBalanceTemperature);
      ++WhiteBalanceTemperature;
      --Focus;
      camera.SetFocus(Focus);
      ++Focus;
      if (camera.GrabFrame() && camera.RetrieveMat(frame))
      {
	 outputVideo << frame;
	 if (!batch)
	    imshow( "Detect", frame);
      } else {
	 fprintf(stderr, "Unable to grab frame from camera.\n");
      }
      if (!batch)
      {
	 wait_key = cv::waitKey(1);
	 if (wait_key == 27 || wait_key == 32)
	    break;
      }
   }
   fprintf(stdout, "Closing camera.\n");
   camera.Close();
}
