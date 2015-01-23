#include <sys/types.h>
#include <dirent.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>

#include "cv.h"
#include "cvaux.h"
#include "cxcore.h"
#include "highgui.h"
using namespace std;
#include "opencvx/cvrect32f.h"
#include "opencvx/cvdrawrectangle.h"
#include "opencvx/cvcropimageroi.h"
#include "opencvx/cvpointnorm.h"
using namespace cv;

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
       if (!item.empty())
        elems.push_back(item);
    }
    return elems;
}


vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}
const double targetAR = 1.5;

int main(void)
{
   DIR *dirp = opendir(".");
   struct dirent *dp;
   vector<string> image_names;
   if (!dirp)
      return -1;

   while ((dp = readdir(dirp)) != NULL) {
      if (strstr(dp->d_name, ".png") && !strstr(dp->d_name, "_r.png"))
	 image_names.push_back(dp->d_name);
   }
   closedir(dirp);
   cout << "Read " << image_names.size() << " image names" << endl;

   const char *w_name = "1";
   const char *miniw_name = "2";
   CvPoint shear = Point(0,0);
   //cvNamedWindow( w_name, CV_WINDOW_AUTOSIZE );
   //cvNamedWindow( miniw_name, CV_WINDOW_AUTOSIZE );
   for (vector<string>::iterator it = image_names.begin(); it != image_names.end(); ++it)
   {
      int frame;
      int rotation;
      Rect rect;
      *it = it->substr(0, it->rfind('.'));
      cerr << *it + "_r.png" << endl;
      vector<string> tokens = split(*it, '_');
      frame       = atoi(tokens[1].c_str());
      rotation    = atoi(tokens[2].c_str());
      rect.x      = atoi(tokens[3].c_str());
      rect.y      = atoi(tokens[4].c_str());
      rect.width  = atoi(tokens[5].c_str());
      rect.height = atoi(tokens[6].c_str());
      CvCapture *cap = cvCaptureFromFile( tokens[0].c_str() );
      cvSetCaptureProperty( cap, CV_CAP_PROP_POS_FRAMES, frame - 1 );
      IplImage *img = cvQueryFrame( cap );
      if( img == NULL )
      {
	 cerr << "Can not open " << tokens[0] << endl;
	 cvReleaseCapture(&cap);
	 continue;
      }
      //cvDrawRectangle(img,cvRect32fFromRect(rect, rotation), cvPointTo32f(shear)); 
      double ar = rect.width / (double)rect.height;
      int added_height = 0;
      int added_width  = 0;
      if (ar > targetAR)
      {
	 added_height = rect.width / targetAR - rect.height;
	 rect.x -= (added_height/ 2.0) * sin(rotation / 180.0 * M_PI);
	 rect.y -= (added_height/ 2.0) * cos(rotation / 180.0 * M_PI);
      }
      else
      {
	 added_width = rect.height * targetAR - rect.width;
	 rect.x -= (added_width / 2.0) * cos(rotation / 180.0 * M_PI);
	 rect.y += (added_width / 2.0) * sin(rotation / 180.0 * M_PI);
      }
      rect.width  += added_width;
      rect.height += added_height;
      
#if 0
      //cout << rect.width << " " << rect.height << " " << added_width << " "<< added_height <<endl;

      cvShowImageAndRectangle( w_name, img, 
	    cvRect32fFromRect( rect, rotation), 
	    cvPointTo32f( shear )); 
      cvShowCroppedImage( miniw_name, img, 
	    cvRect32fFromRect( rect, rotation ), 
	    cvPointTo32f( shear ));
#endif
     
      IplImage* crop = cvCreateImage( 
	    cvSize( rect.width, rect.height ), 
	    img->depth, img->nChannels );
      cvCropImageROI( img, crop, 
	    cvRect32fFromRect( rect, rotation ), 
	    cvPointTo32f( shear ) );

      cvSaveImage( (*it + "_r.png").c_str(), crop );
      cvReleaseImage( &crop );
      cvReleaseCapture(&cap);
   }
   return 0;
}
