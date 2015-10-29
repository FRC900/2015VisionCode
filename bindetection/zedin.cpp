#include "zedin.hpp"
#include <iostream>

using namespace cv;
using namespace std;


ZedIn::ZedIn()
{
	sl::zed::Camera* zed = new sl::zed::Camera(sl::zed::VGA);
	// init computation mode of the zed
	sl::zed::ERRCODE err = zed->init(sl::zed::MODE::QUALITY, -1, true);
	cout << sl::zed::errcode2str(err) << endl;
	// Quit if an error occurred
	if (err != sl::zed::SUCCESS) {
    		delete zed;
    		exit(-1);
	}
	int width = zed->getImageSize().width;
	int height = zed->getImageSize().height;
	_frame = Mat(height, width, CV_8UC4);
}

bool ZedIn::getNextFrame(cv::Mat &frame,bool pause,bool left)
{
   if (!pause)
   {
      bool res = zed->grab(sl::zed::FULL);
      if(left) {
      	imageGPU = zed->getView_gpu(sl::zed::STEREO_LEFT);
      }else{
	imageGPU = zed->getView_gpu(sl::zed::STEREO_RIGHT);
      }
      cerr << "Hi" << endl;
      depthMat = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH);
      cudaMemcpy2D((uchar*) _frame.data, _frame.step, (Npp8u*) imageGPU.data, imageGPU.step, imageGPU.getWidthByte(), imageGPU.height, cudaMemcpyDeviceToHost);
   }
   frame = _frame.clone();
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

