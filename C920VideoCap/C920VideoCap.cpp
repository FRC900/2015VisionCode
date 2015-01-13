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

/*  Four-character-code (FOURCC) */
#define fourcc(a,b,c,d)\
   (((__u32)(a)<<0)|((__u32)(b)<<8)|((__u32)(c)<<16)|((__u32)(d)<<24))

int main(int argc, char* argv[]) {
   //setenv("DISPLAY", ":0", 0);
   fprintf(stdout, "Preparing to open camera.\n");
   camera.Open("/dev/video1");
   if (!camera.IsOpen()) {
      fprintf(stderr, "Unable to open camera.\n");
      return -1;
   }
   Size S(800, 600);
   int fourCC = fourcc('M','J','P','G');

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

   VideoWriter outputVideo(name, fourCC, 30, S, true);
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
   camera.GetFocus(Focus);
   ++Focus;
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

   // Load Face cascade (.xml file)
   CascadeClassifier face_cascade;
   face_cascade.load( "classifier_bin_1/cascade.xml" );

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
#if 0
	 // Detect faces
	 std::vector<Rect> faces;
	 face_cascade.detectMultiScale( frame, faces, 1.1, 2, 0|CV_HAAR_SCALE_IMAGE, Size(30, 30) );

	 // Draw circles on the detected faces
	 for( size_t i = 0; i < faces.size(); i++ )
	 {
	    Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
	    ellipse( frame, center, Size( faces[i].width*0.5, faces[i].height*0.5), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
	 }
#endif
	 imshow( "Detect", frame);
      } else {
	 fprintf(stderr, "Unable to grab frame from camera.\n");
      }
      wait_key = cv::waitKey(5);
      if (wait_key == 27 || wait_key == 32)
	 break;
   }
   fprintf(stdout, "Closing camera.\n");
   camera.Close();
}
