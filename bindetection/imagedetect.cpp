#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/opencv.hpp>
#include "imagedetect.hpp"

#include "iostream"
//#define DETECT_ROTATED

int scale         = 10;
int neighbors     = 5;
int minDetectSize = 20;
int maxDetectSize = 200 * 4;
int gpuScale      = 99;


// TODO : make this a parameter to the detect code
// so that we can detect objects with different aspect ratios
const double DETECT_ASPECT_RATIO = 1.0;

using namespace std;
using namespace cv;
using namespace cv::gpu;

// Take an input image. Threshold it so that pixels within
// the HSV range specified by [HSV]_[MIN,MAX] are set to non-zero
// and the rest of the image is set to zero. Apply a morph
// open to the resulting image
static void generateThreshold(const Mat &ImageIn, Mat &ImageOut,
	      int H_MIN, int H_MAX, int S_MIN, int S_MAX, int V_MIN, int V_MAX)
{
   Mat ThresholdLocalImage;
   vector<Mat> SplitImage;
   Mat SplitImageLE;
   Mat SplitImageGE;

   cvtColor(ImageIn, ThresholdLocalImage, CV_BGR2HSV, 0);
   split(ThresholdLocalImage, SplitImage);
   int max[3] = {H_MAX, S_MAX, V_MAX};
   int min[3] = {H_MIN, S_MIN, V_MIN};
   for (size_t i = 0; i < SplitImage.size(); i++)
   {
      compare(SplitImage[i], min[i], SplitImageGE, cv::CMP_GE);
      compare(SplitImage[i], max[i], SplitImageLE, cv::CMP_LE);
      bitwise_and(SplitImageGE, SplitImageLE, SplitImage[i]);
   }
   bitwise_and(SplitImage[0], SplitImage[1], ImageOut);
   bitwise_and(SplitImage[2], ImageOut, ImageOut);

   Mat erodeElement (getStructuringElement( MORPH_RECT,Size(3,3)));
   //dilate with larger element to make sure object is nicely visible
   Mat dilateElement(getStructuringElement( MORPH_ELLIPSE,Size(11,11)));
   erode(ImageOut, ImageOut, erodeElement, Point(-1,-1), 2);
   dilate(ImageOut, ImageOut, dilateElement, Point(-1,-1), 2);
}

void thresholdImage(const Mat &frame, Mat &outFrame, vector <Rect> &rects,
	      int H_MIN, int H_MAX, int S_MIN, int S_MAX, int V_MIN, int V_MAX)
{
   // Threshold the image using supplied HSV value
   vector < vector <Point> > contours;
   rects.clear();

   generateThreshold(frame, outFrame, H_MIN, H_MAX, S_MIN, S_MAX, V_MIN, V_MAX);
   Mat tempFrame = outFrame.clone();

   findContours(tempFrame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

   // Find contours in the thresholded image.  Grab bounding
   // rectangles around those. Ideally, if the HSV values are 
   // set correctly, these rectangles will be the only
   // places that green-enough objects are found
   for (size_t i = 0; i < contours.size(); i++)
   {
      Rect rect = boundingRect(contours[i]);
      // Scale up rectangle a bit so that the
      // bounds are larger than each green blob detected
      const double RECT_SCALE_UP = 0.33;
      rect.x -= rect.height / (1/(RECT_SCALE_UP/2));
      rect.y -= rect.width / (1/(RECT_SCALE_UP/2));
      rect.height *= (1.0 + RECT_SCALE_UP);
      rect.width *= (1.0 + RECT_SCALE_UP);
      // Save bounding rects which are large enough
      // to actually contain the smallest image 
      // searched for
      if ((rect.width > (minDetectSize * DETECT_ASPECT_RATIO))&&(rect.height > minDetectSize))
	 rects.push_back(rect);
   }
}
/**
 * Rotate an image
 */
static void rotate(const Mat& src, double angle, Mat& dst)
{
    int len = std::max(src.cols, src.rows);
    Point2f pt(len/2., len/2.);
    Mat r = getRotationMatrix2D(pt, angle, 1.0);

    warpAffine(src, dst, r, Size(len, len));
}

static void DrawRects(string windowName, const Mat &frame, Rect *faces, size_t facesCount)
{
   Mat frameBGR;
   cvtColor(frame, frameBGR, CV_GRAY2BGR);
   for (size_t i = 0; i < facesCount; i++)
   {
      cout << windowName << " " << faces[i].x << " " << faces[i].y << " " << faces[i].width << " " << faces[i]. height << endl;
      rectangle(frameBGR, faces[i], Scalar(0,255,255), 3);
   }
   imshow(windowName, frameBGR);
}

const int rotateCount = 4;
void CPU_CascadeDetect::cascadeDetect(const Mat &frame, 
                                      vector<Rect> &imageRects, 
                                      vector<unsigned> &direction)
{
  Mat frameGray;
  Mat frameRotated[rotateCount];
  cvtColor( frame, frameGray, CV_BGR2GRAY );
  equalizeHist( frameGray, frameRotated[0]);
  frameGray.release();

#ifdef DETECT_ROTATED
  rotate(frameRotated[0],  90, frameRotated[1]);
  frameRotated[1] = frameRotated[1](Rect(0, 0, frame.rows, frame.cols));

  rotate(frameRotated[0], 180, frameRotated[2]);
  frameRotated[2] = frameRotated[2](Rect(0, fabs(frame.cols - frame.rows), frame.cols, frame.rows));

  rotate(frameRotated[0], 270, frameRotated[3]);
  frameRotated[3] = frameRotated[3](Rect(fabs(frame.rows - frame.cols), 0, frame.rows, frame.cols));
#endif

  //-- Detect faces
  _classifier.detectMultiScale(frameRotated[0], 
	imageRects, 
	1.05 + scale/100., 
	neighbors, 
	0|CV_HAAR_SCALE_IMAGE, 
	Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize), 
	Size(maxDetectSize * DETECT_ASPECT_RATIO, maxDetectSize) );
  direction.clear();
  for (size_t i = 0; i < imageRects.size(); i++)
     direction.push_back(1);

#ifdef DETECT_ROTATED
  vector<Rect> rects;
  _classifier.detectMultiScale(frameRotated[1], 
	rects,
	1.05 + scale/100., 
	neighbors,
	0|CV_HAAR_SCALE_IMAGE, 
	Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize), 
	Size(maxDetectSize * DETECT_ASPECT_RATIO, maxDetectSize) );
  direction.clear();
  for (size_t i = 0; i < rects.size(); i++)
  {
     imageRects.push_back(Rect(frame.cols - 1 - (rects[i].y + rects[i].height), rects[i].x, rects[i].height, rects[i].width));
     direction.push_back(2);
  }

#if 0
  _classifier.detectMultiScale(frameRotated[2], 
	rects,
	1.05 + scale/100., 
	neighbors,
	0|CV_HAAR_SCALE_IMAGE, 
	Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize), 
	Size(maxDetectSize * DETECT_ASPECT_RATIO, maxDetectSize) );
  direction.clear();
  for (size_t i = 0; i < rects.size(); i++)
  {
     imageRects.push_back(Rect(frame.cols - 1 - (rects[i].x + rects[i].height), frame.rows - 1 - (rects[i].y + rects[i].height), rects[i].width, rects[i].height));
     direction.push_back(4);
  }
#endif

  _classifier.detectMultiScale(frameRotated[3], 
	rects,
	1.01 + scale/100., 
	neighbors,
	0|CV_HAAR_SCALE_IMAGE, 
	Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize), 
	Size(maxDetectSize * DETECT_ASPECT_RATIO, maxDetectSize) );
  direction.clear();
  for (size_t i = 0; i < rects.size(); i++)
  {
     imageRects.push_back(Rect(rects[i].y, frame.rows - 1 - (rects[i].x + rects[i].width), rects[i].height, rects[i].width));
     direction.push_back(8);
  }
#endif
}

static void DrawRects(string windowName, const GpuMat &frameGPU, Rect *faces, size_t facesCount)
{
   Mat frame;

   frameGPU.download(frame);
   cvtColor(frame, frame, CV_GRAY2BGR);
   for (size_t i = 0; i < facesCount; i++)
   {
      cout << windowName << " " << faces[i].x << " " << faces[i].y << " " << faces[i].width << " " << faces[i]. height << endl;
      rectangle(frame, faces[i], Scalar(0,255,255), 3);
   }
   imshow(windowName, frame);
}
Rect scaleRects(const Rect &input, float inputScaleFactor) {
  float scaleFactor = 1.0 / inputScaleFactor;
  return Rect((input.x * scaleFactor),(input.y * scaleFactor),(input.width * scaleFactor),(input.height * scaleFactor));
}

void GPU_CascadeDetect::cascadeDetect (const GpuMat &frameGPUInput, vector<Rect> &imageRects, vector<unsigned> &direction) //gpu version
{
  GpuMat detectResultGPU;
  cvtColor(frameGPUInput, frameGray, CV_BGR2GRAY);
  equalizeHist(frameGray, frameGPU[0]);

  direction.clear();
  //-- Detect objects
  int detectCount;
  detectCount = _classifier.detectMultiScale(frameGPU[0], 
	 detectResultGPU, 
	 1.01 + scale/100., 
	 neighbors, 
	 Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize));

  // download only detected number of rectangles
  Mat detectResult;
  Rect* rects;
  detectResultGPU.colRange(0, detectCount).download(detectResult);

  imageRects.clear();
  rects = detectResult.ptr<Rect>();
  for(int i = 0; i < detectCount; ++i)
  {
     imageRects.push_back(rects[i]);
     direction.push_back(1);
  }

#ifdef DETECT_ROTATED
  // rotate 90 degress into frameGPU[1] and 180 into frameGPU[2]
  rotate(frameGPU[0], frameGPU[1], Size(frameGPUInput.rows, frameGPUInput.cols), -90.0, frameGPUInput.rows, 0);
  rotate(frameGPU[0], frameGPU[2], Size(frameGPUInput.cols, frameGPUInput.rows), 180.0, frameGPUInput.cols, frameGPUInput.rows);
  rotate(frameGPU[0], frameGPU[3], Size(frameGPUInput.rows, frameGPUInput.cols),  90.0, 0, frameGPUInput.cols);

  //-- Detect objects at 90 degree rotation
  detectCount = _classifier.detectMultiScale(frameGPU[1],
	detectResultGPU, 
	1.05 + scale/100., 
	neighbors, 
	Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize)); 

  // download only detected number of rectangles
  detectResultGPU.colRange(0, detectCount).download(detectResult);

  rects = detectResult.ptr<Rect>();
  //DrawRects("90", frameGPU[1], rects, detectCount);
  for(int i = 0; i < detectCount; ++i)
  {
     //cout << "90 x " << rects[i].x << " -> " << rects[i].y << endl;
     //cout << "90 y " << rects[i].y << " -> " << frame.rows - 1 - (rects[i].x + rects[i].width) << endl;
     imageRects.push_back(Rect(rects[i].y, frameGPUInput.rows - 1 - (rects[i].x + rects[i].width), rects[i].height, rects[i].width));
     direction.push_back(2);
  }

#if 1
  //-- Detect objects at 180 degree rotation
  detectCount = _classifier.detectMultiScale(frameGPU[2], 
	detectResultGPU, 
	1.05 + scale/100., 
	neighbors, 
	Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize)); 

  // download only detected number of rectangles
  detectResultGPU.colRange(0, detectCount).download(detectResult);

  rects = detectResult.ptr<Rect>();
  //DrawRects("180", frameGPU[2], rects, detectCount);
  for(int i = 0; i < detectCount; ++i)
  {
     //cout << "180 x " << rects[i].x << " -> " << frame.cols - 1 - (rects[i].x + rects[i].height) << endl;
     //cout << "180 y " << rects[i].y << " -> " << frame.rows - 1 - (rects[i].y + rects[i].width) << endl;
     imageRects.push_back(Rect(frameGPUInput.cols - 1 - (rects[i].x + rects[i].height), frameGPUInput.rows - 1 - (rects[i].y + rects[i].height), rects[i].width, rects[i].height));
     direction.push_back(4);
  }
#endif

  //-- Detect objects at -90 degree rotation
  detectCount = _classifier.detectMultiScale(frameGPU[3], 
	detectResultGPU, 
	1.05 + scale/100., 
	neighbors, 
	Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize));

  // download only detected number of rectangles
  detectResultGPU.colRange(0, detectCount).download(detectResult);

  rects = detectResult.ptr<Rect>();
  //DrawRects("-90", frameGPU[3], rects, detectCount);
  for(int i = 0; i < detectCount; ++i)
  {
     //cout << "-90 x " << rects[i].x << " -> " << frame.cols - 1 - (rects[i].y + rects[i].height) << endl;
     //cout << "-90 y " << rects[i].y << " -> " << rects[i].x  << endl;
     imageRects.push_back(Rect(frameGPUInput.cols - 1 - (rects[i].y + rects[i].height), rects[i].x, rects[i].height, rects[i].width));
     direction.push_back(8);
  }
#endif
}

void GPU_CascadeDetect::cascadeDetect (const Mat &frame, vector<Rect> &imageRects, vector<unsigned> &direction) { //gpu version with wrapper
   Mat nonConstFrame = frame.clone(); //create a copy that's not constant
   GpuMat uploadFrame;
   uploadFrame.upload(frame);
   cascadeDetect(uploadFrame, imageRects, direction);
   float fxy = 1.0 / (1.01 + gpuScale/100.0); //create the scale factor
   while(fxy > 0.5)
   {
    Mat resized(round(fxy * frame.cols),round(fxy * frame.rows),frame.type()); //create a target image with same type and different size as original
    resize(nonConstFrame,resized,Size(0,0),fxy,fxy,INTER_LINEAR);
    uploadFrame.upload(resized);
    vector <Rect> frameImageRects;
    vector <unsigned> frameImageDirections;
    cascadeDetect(uploadFrame, frameImageRects, frameImageDirections);
    for(int i = 0; i < frameImageRects.size(); i++) {
      imageRects.push_back(scaleRects(frameImageRects[i], fxy));
      direction.push_back(frameImageDirections[i]);
    }
    fxy = fxy / (1.01 + gpuScale/100.0); // create the scale factor
  }
}

// For each detected rectange, check if each rect is in
// any of the thresholded rectangles. If so, push the detected
// rectangle into filteredRects
void filterUsingThreshold(const vector<Rect> &detectRects,
                          const vector<Rect> &threshRects,
			  vector<Rect> &filteredRects)
{
   filteredRects.clear();
   for(size_t i = 0; i < detectRects.size(); i++)
   {
      // Hightlight detected images which are fully contained in 
      // green contour bounding rectangles
      bool inRect = false;
      for (size_t j = 0; !inRect && (j < threshRects.size()); j++)
      {
	 // If the intersection is the same as the smaller
	 // detected rectangle, the detected rectangle is fully
	 // contained in the threshold rectangle
	 if ((detectRects[i] & threshRects[j]) == detectRects[i])
	    inRect = true;
      }
      if (inRect)
	 filteredRects.push_back(detectRects[i]);
   }
}
