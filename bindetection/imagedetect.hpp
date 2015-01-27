#ifndef INC_IMAGEDETECT_HPP__
#define INC_IMAGEDETECT_HPP__

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/gpu/gpu.hpp>

#include <vector>

class BaseCascadeDetect
{
   public :
      BaseCascadeDetect() : cascadeLoaded(false) {}
      virtual ~BaseCascadeDetect() {}
      virtual void cascadeDetect(const cv::Mat &frame, std::vector<cv::Rect> &imageRects ) =0;
      bool loaded(void)
      {
	 return cascadeLoaded;
      }

   protected:
      bool cascadeLoaded;
};

class CPU_CascadeDetect : public BaseCascadeDetect
{
   public :
      CPU_CascadeDetect(const char *cascadeName) : BaseCascadeDetect()
      {
	 cascadeLoaded = _classifier.load(cascadeName);
      }
      void cascadeDetect(const cv::Mat &frame, std::vector<cv::Rect> &imageRects );

   private :
      cv::CascadeClassifier _classifier;
};

class GPU_CascadeDetect : public BaseCascadeDetect
{
   public :
      GPU_CascadeDetect(const char *cascadeName) : BaseCascadeDetect()
      {
	 cascadeLoaded = _classifier.load(cascadeName);
      }
      void cascadeDetect(const cv::Mat &frame, std::vector<cv::Rect> &imageRects );
      void cascadeDetect(const cv::gpu::GpuMat &frameGPUInput, std::vector<cv::Rect> &imageRects);

   private :
      cv::gpu::CascadeClassifier_GPU _classifier;
      cv::gpu::GpuMat frameGPU[4];
      cv::gpu::GpuMat frameGray;
};

extern int scale;
extern int neighbors;
extern int minDetectSize;
extern int maxDetectSize;
extern int histDivider;

void thresholdImage(const cv::Mat &frame, cv::Mat &outFrame, std::vector <cv::Rect> &rects,
			  int H_MIN, int H_MAX, int S_MIN, int S_MAX, int V_MIN, int V_MAX);
void filterUsingThreshold(const std::vector<cv::Rect> &detectRects,
                          const std::vector<cv::Rect> &threshRects,
			  std::vector<cv::Rect> &filteredRects);
#endif
