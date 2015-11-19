#ifndef INC_IMAGEDETECT_HPP__
#define INC_IMAGEDETECT_HPP__

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/gpu/gpu.hpp>

#include <vector>

// Base class for detector. Doesn't really do much - all of the heavy lifting is
// in the derived classes
class BaseCascadeDetect
{
   public :
      BaseCascadeDetect() : cascadeLoaded(false) {} //pass in value of false to cascadeLoaded
      virtual ~BaseCascadeDetect() {} //empty destructor
      virtual void cascadeDetect(const cv::Mat &frame, std::vector<cv::Rect> &imageRects) = 0; //pure virtual function, must be defined by CPU and GPU detect
      virtual void cascadeDetect(const cv::gpu::GpuMat &frameGPUInput, std::vector<cv::Rect> &imageRects) 
      {
	 imageRects.clear();
      }
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
      CPU_CascadeDetect(const char *cascadeName) : BaseCascadeDetect() // call default constructor of base class
      {
	 cascadeLoaded = classifier_.load(cascadeName);
      }
      ~CPU_CascadeDetect(void)
      {
      }
      void cascadeDetect(const cv::Mat &frame, std::vector<cv::Rect> &imageRects); //defined elsewhere

   private :
      cv::CascadeClassifier classifier_;
};

class GPU_CascadeDetect : public BaseCascadeDetect
{
   public :
      GPU_CascadeDetect(const char *cascadeName) : BaseCascadeDetect()
      {
	 cascadeLoaded = classifier_.load(cascadeName);
      }
      ~GPU_CascadeDetect(void)
      {
	 classifier_.release();
      }
      void cascadeDetect (const cv::Mat          &frame,        std::vector<cv::Rect> &imageRects);
      void cascadeDetect (const cv::gpu::GpuMat &frameGPUInput, std::vector<cv::Rect> &imageRects);

   private :
      cv::gpu::CascadeClassifier_GPU classifier_;

      // Declare GPU Mat elements once here instead of every single
      // call to the functions which use them
      cv::gpu::GpuMat frameEq;
      cv::gpu::GpuMat frameGray;
      cv::gpu::GpuMat detectResultsGPU;
      cv::gpu::GpuMat uploadFrame;
};

extern int scale;
extern int neighbors;
extern int minDetectSize;
extern int maxDetectSize;

#endif
