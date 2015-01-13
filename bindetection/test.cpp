 #include "opencv2/objdetect/objdetect.hpp"
 #include "opencv2/highgui/highgui.hpp"
 #include "opencv2/imgproc/imgproc.hpp"

 #include <iostream>
 #include <stdio.h>

 using namespace std;
 using namespace cv;

 /** Function Headers */
 void detectAndDisplay( Mat frame );

 /** Global variables */
 String face_cascade_name = "cascade.xml";
 CascadeClassifier face_cascade;
 string window_name = "Capture - Face detection";

 int scale     = 20;
 int neighbors = 30;
 int min_row   = 40;
 int min_col   = 50;
 int max_row   = 400 * 4;
 int max_col   = 500 * 4;
 int r_min     = 65;
 int r_max     = 90;
 int b_min     = 100;
 int b_max     = 170;
 int hist_divider = 1;
 /** @function main */
int main( int argc, const char** argv )
{
   VideoCapture cap(0);
   Mat frame;
   Mat frame_copy;

   bool pause = false;

   namedWindow("Parameters", cv::WINDOW_AUTOSIZE);
   createTrackbar ("Scale", "Parameters", &scale, 200, NULL);
   createTrackbar ("Neighbors", "Parameters", &neighbors, 500, NULL);
  // createTrackbar ("Min Row", "Parameters", &min_row, 1000, NULL);
  // createTrackbar ("Min Col", "Parameters", &min_col, 1000, NULL);
  // createTrackbar ("Max Row", "Parameters", &max_row, 1000, NULL);
  // createTrackbar ("Max Col", "Parameters", &max_col, 1000, NULL);
   createTrackbar ("R Min", "Parameters", &r_min, 256, NULL);
   createTrackbar ("R Max", "Parameters", &r_max, 256, NULL);
   createTrackbar ("B Min", "Parameters", &b_min, 256, NULL);
   createTrackbar ("B Max", "Parameters", &b_max, 256, NULL);
   createTrackbar ("Hist Divider", "Parameters", &hist_divider, 16, NULL);

   //-- 1. Load the cascades
   if( !face_cascade.load( face_cascade_name ) ){ printf("--(!)Error loading\n"); return -1; };

   //-- 2. Read the video stream
   while( true )
   {
      if (!pause)
      {
	 cap >> frame;
	 frame_copy = frame.clone();
	
      }
      else
	 frame = frame_copy.clone();
         pyrDown(frame, frame);

      //-- 3. Apply the classifier to the frame
      if( !frame.empty() )
      { detectAndDisplay( frame ); }
      else
      { printf(" --(!) No captured frame -- Break!"); break; }
      int c = waitKey(1);
      if( (char)c == 'c' ) { break; }
      if( c == ' ') pause = !pause;
   }
      return 0;
 }
string type2str(int type) {
  string r;

  uchar depth = type & CV_MAT_DEPTH_MASK;
  uchar chans = 1 + (type >> CV_CN_SHIFT);

  switch ( depth ) {
    case CV_8U:  r = "8U"; break;
    case CV_8S:  r = "8S"; break;
    case CV_16U: r = "16U"; break;
    case CV_16S: r = "16S"; break;
    case CV_32S: r = "32S"; break;
    case CV_32F: r = "32F"; break;
    case CV_64F: r = "64F"; break;
    default:     r = "User"; break;
  }

  r += "C";
  r += (chans+'0');

  return r;
}

 float range[] = { 0, 256 } ;
  const float* histRange = { range };
  bool uniform = true; bool accumulate = false;
/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
  std::vector<Rect> faces;
  Mat frame_gray;
int histSize = 256 / (hist_divider ? hist_divider : 1);

  cvtColor( frame, frame_gray, CV_BGR2GRAY );
  equalizeHist( frame_gray, frame_gray );

  //-- Detect faces
  face_cascade.detectMultiScale( frame_gray, faces, std::max(1.05,(double)scale/20.), neighbors, 0|CV_HAAR_SCALE_IMAGE, Size(min_row, min_col), Size(max_row, max_col) );

  for( size_t i = 0; i < faces.size(); i++ )
  {
    vector<Mat> bgr_planes;
    Mat smallImg;
    stringstream winName;
   winName << "Face "; 
    winName << i;
    string winNameStr = winName.str();
    frame(Rect(faces[i].x, faces[i].y + faces[i].height*1/2, faces[i].width, faces[i].height/2)).copyTo(smallImg);
    split (smallImg, bgr_planes);
    bool uniform = true; bool accumulate = false;

    //cvtColor(smallImg, smallImg, COLOR_BGR2HSV);
    Mat hist[3];
    double min_val[3];
    double max_val[3];
    int min_idx[3];
    int max_idx[3];

    for (size_t j = 0; j < 3; j++)
    {
       /// Compute the histograms:
       calcHist( &bgr_planes[j], 1, 0, Mat(), hist[j], 1, &histSize, &histRange, uniform, accumulate );

       hist[j].at<float>(255) = 0.;
       hist[j].at<float>(0) = 0.;
       Point min, max;
       minMaxLoc(hist[j], min_val, max_val + j, &min, &max);
       min_idx[j] = min.y;
       max_idx[j] = max.y;
       //cout << i << " " << type2str(hist[j].type()) << " " << j << " " << max_idx[j] << endl;
	
    }
/*
double ratioBG, ratioBR, ratioGR;
ratioBG = (double)max_idx[0] / max_idx[1];
ratioBR = (double)max_idx[0] / max_idx[2];
ratioGR = (double)max_idx[1] / max_idx[2];
cout << "BG Ratio: " << ratioBG;
cout << "   BR Ratio: " << ratioBR;
cout << "   GR Ratio: " << ratioGR << endl;
if (max_idx[0] > max_idx[2] && max_idx[1] > max_idx[2])
rectangle( frame, faces[i], Scalar( 0, 0, 255 ),3);

*/
    if ((max_idx[0] + max_idx[1]) < max_idx[2])
    {
       cout << "R hit " ;
       for (size_t j = 0; j < 3; j++)
	  cout << " " << max_idx[j];
       cout << endl;
       rectangle( frame, faces[i], Scalar( 0, 0, 255 ),3);
    }
    else if ((max_idx[1] + max_idx[2]) < max_idx[0])
    {
       cout << "B hit " ;
       for (size_t j = 0; j < 3; j++)
	  cout << " " << max_idx[j];
       cout << endl;
       rectangle( frame, faces[i], Scalar( 255, 0, 0 ),3);
    }
    else if ((std::min(max_idx[0], max_idx[1]) * 1.5) < max_idx[2])    
    {
       cout << "R2 hit " ;
       for (size_t j = 0; j < 3; j++)
	  cout << " " << max_idx[j];
       cout << endl;
       rectangle( frame, faces[i], Scalar( 0, 128, 255 ),3);
    }
    else if ((std::min(max_idx[1], max_idx[2]) * 1.5) < max_idx[0])    
    {
       cout << "B2 hit " ;
       for (size_t j = 0; j < 3; j++)
	  cout << " " << max_idx[j];
       cout << endl;
       rectangle( frame, faces[i], Scalar( 255, 128, 0 ),3);
    }

    else
    rectangle( frame, faces[i], Scalar( 255, 0, 255 ),3);

#if 1
    // Draw the histograms for B, G and R
    int hist_w = 512; int hist_h = 400;
    int bin_w = cvRound( (double) hist_w/histSize );

    Mat histImage( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
    /// Normalize the result to [ 0, histImage.rows ]
    normalize(hist[0], hist[0], 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(hist[1], hist[1], 0, histImage.rows, NORM_MINMAX, -1, Mat() );
    normalize(hist[2], hist[2], 0, histImage.rows, NORM_MINMAX, -1, Mat() );

    /// Draw for each channel
    for( int ii = 1; ii < histSize; ii++ )
    {
       line( histImage, Point( bin_w*(ii-1), hist_h - cvRound(hist[0].at<float>(ii-1)) ) ,
	     Point( bin_w*(ii), hist_h - cvRound(hist[0].at<float>(ii)) ),
	     Scalar( 255, 0, 0), 2, 8, 0  );
       line( histImage, Point( bin_w*(ii-1), hist_h - cvRound(hist[1].at<float>(ii-1)) ) ,
	     Point( bin_w*(ii), hist_h - cvRound(hist[1].at<float>(ii)) ),
	     Scalar( 0, 255, 0), 2, 8, 0  );
       line( histImage, Point( bin_w*(ii-1), hist_h - cvRound(hist[2].at<float>(ii-1)) ) ,
	     Point( bin_w*(ii), hist_h - cvRound(hist[2].at<float>(ii)) ),
	     Scalar( 0, 0, 255), 2, 8, 0  );
    }
    namedWindow(winNameStr, WINDOW_NORMAL);
    imshow (winNameStr, smallImg);
    winName << " Hist";
    imshow (winName.str(), histImage);
#endif

    if (i > 20) break;
  }
#if 0
  for( size_t i = 0; i < faces.size(); i++ )
  {
    //Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 );
    rectangle( frame, faces[i], Scalar( 255, 0, 255 ),3);
  }
#endif
  //-- Show what you got
  imshow( window_name, frame );
 }

