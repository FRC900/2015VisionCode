#ifndef INC_IMAGEDETECT_HPP__
#define INC_IMAGEDETECT_HPP__

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include <vector>


extern int H_MIN;
extern int H_MAX;
extern int S_MIN;
extern int S_MAX;
extern int V_MIN;
extern int V_MAX;

extern int scale;
extern int neighbors;
extern int minDetectSize;
extern int maxDetectSize;
//int r_min     = 65;
//int r_max     = 90;
//int b_min     = 100;
//int b_max     = 170;
extern int hist_divider;

void thresholdImage(const cv::Mat &frame, cv::Mat &outFrame, std::vector <cv::Rect> &rects);
void cascadeDetect ( const cv::Mat &frame, cv::CascadeClassifier &face_cascade, std::vector<cv::Rect> &imageRects );
void filterUsingThreshold (const std::vector<cv::Rect> &detectRects,
                           const std::vector<cv::Rect> &threshRects,
			   std::vector<cv::Rect> filteredRects);
#endif
