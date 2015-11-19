#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <opencv2/gpu/gpu.hpp>
#include <opencv2/opencv.hpp>
#include "imagedetect.hpp"

#include "iostream"

int scale         =  4;
int neighbors     = 4;
int minDetectSize = 20;
int maxDetectSize = 450;

// TODO : make this a parameter to the detect code
// so that we can detect objects with different aspect ratios
const double DETECT_ASPECT_RATIO = 1.0;

using namespace std;
using namespace cv;
using namespace cv::gpu;

void CPU_CascadeDetect::cascadeDetect (const Mat &frame, 
                                       vector<Rect> &imageRects)
{
  Mat frameGray;
  Mat frameEq;
  cvtColor( frame, frameGray, CV_BGR2GRAY );
  equalizeHist( frameGray, frameEq);

  classifier_.detectMultiScale(frameEq, 
	imageRects, 
	1.01 + scale/100., 
	neighbors, 
	0|CV_HAAR_SCALE_IMAGE, 
	Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize), 
	Size(maxDetectSize * DETECT_ASPECT_RATIO, maxDetectSize) );
}


void GPU_CascadeDetect::cascadeDetect (const GpuMat &frameGPUInput, vector<Rect> &imageRects) 
{
  cvtColor(frameGPUInput, frameGray, CV_BGR2GRAY);
  equalizeHist(frameGray, frameEq);

  //-- Detect objects
  int detectCount;
  detectCount = classifier_.detectMultiScale(frameEq, 
	 detectResultsGPU, 
	 Size(maxDetectSize * DETECT_ASPECT_RATIO, maxDetectSize),
	 Size(minDetectSize * DETECT_ASPECT_RATIO, minDetectSize),
	 1.01 + scale/100., 
	 neighbors);

  // download only detected number of rectangles
  Mat detectResult;
  detectResultsGPU.colRange(0, detectCount).download(detectResult);

  imageRects.clear();
  Rect *rects = detectResult.ptr<Rect>();
  for(int i = 0; i < detectCount; ++i)
     imageRects.push_back(rects[i]);
}

//gpu version with wrapper
void GPU_CascadeDetect::cascadeDetect (const Mat &frame, vector<Rect> &imageRects) 
{
   uploadFrame.upload(frame);
   cascadeDetect(uploadFrame, imageRects);
}
