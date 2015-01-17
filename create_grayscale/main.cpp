#include <sys/types.h>
#include <dirent.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>
#include <stdio.h>

using namespace cv;
using namespace std;

int main(void)
{
   DIR *dirp = opendir(".");
   struct dirent *dp;

   while (dirp) {
      if ((dp = readdir(dirp)) != NULL) {

	 if (strstr(dp->d_name, ".png") || strstr(dp->d_name, ".jpg") ){
	    if (strstr(dp->d_name, "_g.png") || strstr(dp->d_name, "_g.jpg"))
	       continue;
	    Mat frame = imread(dp->d_name);

	    string str(dp->d_name);
	    //cerr << str << endl;
	    //cerr << str.rfind('.') << endl;
	    str = str.substr(0, str.rfind('.'));
	    cerr << str + "_g.png" << endl;
	    // Save grayscale equalized version
	    Mat frameGray;
	    cvtColor( frame, frameGray, CV_BGR2GRAY );
	    equalizeHist( frameGray, frameGray );
	    imwrite(str + "_g.png", frameGray);
	 }
      } 
   }
   closedir(dirp);
}
