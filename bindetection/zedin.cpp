#include "zedin.hpp"

using namespace cv;
using namespace std;


ZedIn::ZedIn()
{
	zed = new sl::zed::Camera(sl::zed::VGA);
	// init computation mode of the zed
	sl::zed::ERRCODE err = zed->init(sl::zed::MODE::QUALITY, -1, true);
	cout << sl::zed::errcode2str(err) << endl;
	// Quit if an error occurred
	if (err != sl::zed::SUCCESS) {
    		delete zed;
    		exit(-1);
	}
	width = zed->getImageSize().width;
	height = zed->getImageSize().height;
}

bool ZedIn::getNextFrame(cv::Mat &frame,bool pause,bool left)
{
   if (!pause)
   {
      zed->grab(sl::zed::FULL);
      if(left) {
      	imageGPU = zed->getView_gpu(sl::zed::STEREO_LEFT);
      }else{
	imageGPU = zed->getView_gpu(sl::zed::STEREO_RIGHT);
      }
      cv::Mat imageCPU(height, width, CV_8UC4);
      depthMat = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH);
      cudaMemcpy2D((uchar*) imageCPU.data, imageCPU.step, (Npp8u*) imageGPU.data, imageGPU.step, imageGPU.getWidthByte(), imageGPU.height, cudaMemcpyDeviceToHost);
      cvtColor(imageCPU,imageCPU,CV_RGBA2RGB);
      frame = imageCPU.clone();
   }else{
      frame = _frame.clone();
   }
   return true;
}

bool ZedIn::getNextFrame(bool pause, cv::Mat &frame) {
	return getNextFrame(frame,pause,true);
	
}

double ZedIn::getDepth(int x, int y) {

	float* data = (float*) depthMat.data;
	float* ptr_image_num = (float*) ((int8_t*) data + y * depthMat.step);
	float dist = ptr_image_num[x] / 1000.f;
	return dist;
}

