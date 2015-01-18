#include <sys/types.h>
#include <dirent.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <string>
#include <stdio.h>
#include <vector>

using namespace cv;
using namespace std;

int main(void)
{
   DIR *dirp = opendir(".");
   struct dirent *dp;
   vector<string> image_names;
   if (!dirp)
      return -1;

   while ((dp = readdir(dirp)) != NULL) {
      if (strstr(dp->d_name, ".png") || strstr(dp->d_name, ".jpg") ){
	 if (strstr(dp->d_name, "_g.png") || strstr(dp->d_name, "_g.jpg"))
	    continue;
	 image_names.push_back(dp->d_name);
      }
   }
   closedir(dirp);
   cout << "Read " << image_names.size() << " image names" << endl;

   for (vector<string>::iterator it = image_names.begin(); it != image_names.end(); ++it)
   {
      Mat frame = imread(*it);

      //cerr << str << endl;
      //cerr << str.rfind('.') << endl;
      *it = it->substr(0, it->rfind('.'));
      cerr << *it + "_g.png" << endl;
      // Save grayscale equalized version
      Mat frameGray;
      cvtColor( frame, frameGray, CV_BGR2GRAY );
      equalizeHist( frameGray, frameGray );
      imwrite(*it + "_g.png", frameGray);
   }
   return 0;
}
