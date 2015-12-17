// Temp file to save histogram code in case we need it
//
int histDivider = 1;
// For a given frame, find the pixels values for each channel with 
// the highest intensities
void generateHistogram(const Mat &frame, double *minIdx, double *maxIdx)
{
   int histSize = 256 / (histDivider ? histDivider : 1);
   float range[] = { 0, 256 } ;
   const float* histRange = { range };
   bool uniform = true, accumulate = false;

   // Split into individual B,G,R channels so we can run a histogram on each
   vector<Mat> bgrPlanes;
   split (frame, bgrPlanes);

   //cvtColor(images[i], images[i], COLOR_BGR2HSV);
   Mat hist[3];
   double minVal[3];
   double maxVal[3];

   for (size_t i = 0; i < 3; i++)
   {
      /// Compute the histograms:
      calcHist(&bgrPlanes[i], 1, 0, Mat(), 
	    hist[i], 1, &histSize, &histRange, uniform, accumulate );

      // Remove 0 & 255 intensities since these seem to confuse things later
      hist[i].at<float>(0)   = 0.;
      hist[i].at<float>(255) = 0.;

      // Grab the color intensity peak
      Point min, max;
      minMaxLoc(hist[i], minVal + i, maxVal + i, &min, &max);
      minIdx[i] = min.y;
      maxIdx[i] = max.y;
   }
}

// Draw the histograms (3 mono channels in hist[0..2]
// on the supplied image
void drawHistogram(const Mat hist[], Mat &histImage); 
{
   int histSize = 256 / (histDivider ? histDivider : 1);
   int hist_w = 512; int hist_h = 400;
   int bin_w = cvRound( (double) hist_w/histSize );

   histImage = Mat( hist_h, hist_w, CV_8UC3, Scalar( 0,0,0) );
   /// Normalize the result to [ 0, histImage.rows ]
   for (size_t j = 0; j < 3; j++)
      normalize(hist[j], hist[j], 0, histImage.rows, NORM_MINMAX, -1, Mat() );

   const Scalar colors[3] = {Scalar(255,0,0), Scalar(0,255,0), Scalar(0,0,255)};
   // For each point in the histogram
   for( int ii = 1; ii < histSize; ii++ )
   {
      // Draw for each channel
      for (size_t jj = 0; jj < 3; jj++)
	 line( histImage, Point( bin_w*(ii-1), hist_h - cvRound(hist[jj].at<float>(ii-1)) ) ,
	       Point( bin_w*(ii), hist_h - cvRound(hist[jj].at<float>(ii)) ),
	       colors[jj], 2, 8, 0  );
   }
}

// Convert type value to a human readable string. Useful for debug
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

