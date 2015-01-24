#ifndef INC_IMAGEDETECT_HPP__
#define INC_IMAGEDETECT_HPP__

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/objdetect/objdetect.hpp"

#include <vector>

extern int scale;
extern int neighbors;
extern int minDetectSize;
extern int maxDetectSize;
extern int histDivider;

void thresholdImage(const cv::Mat &frame, cv::Mat &outFrame, std::vector <cv::Rect> &rects,
			  int H_MIN, int H_MAX, int S_MIN, int S_MAX, int V_MIN, int V_MAX);
void cascadeDetect(const cv::Mat &frame, cv::CascadeClassifier &face_cascade, std::vector<cv::Rect> &imageRects );
void filterUsingThreshold(const std::vector<cv::Rect> &detectRects,
                          const std::vector<cv::Rect> &threshRects,
			  std::vector<cv::Rect> &filteredRects);
#endif
