#include "videoin.hpp"
#include <iostream>

using namespace cv;
using namespace std;


VideoIn::VideoIn()
{
	Camera* zed = new Camera(VGA);
	// init computation mode of the zed
	ERRCODE err = zed->init(MODE::QUALITY, -1, true);
	cout << errcode2str(err) << endl;
	// Quit if an error occurred
	if (err != SUCCESS) {
    		delete zed;
    		return -1;
	}
}

bool VideoIn::getFrame(bool pause,bool left, Mat &frame)
{
   if (!pause)
   {
      if(left) {
      	sl::zed::Mat imageGPU = zed->getView_gpu(STEREO_LEFT);
      }else{
	sl::zed::Mat imageGPU = zed->getView_gpu(STEREO_RIGHT);
      }
      cudaMemcpy2D((uchar*) _frame.data, _frame.step, (Npp8u*) imageGPU.data, imageGPU.step, imageGPU.getWidthByte(), imageGPU.height, cudaMemcpyDeviceToHost);
   }
   frame = _frame.clone();
   return true;
}

double VideoIn::getDepth(bool pause, int x, int y) {

	sl::zed::Mat input = zed->retrieveMeasure(sl::zed::MEASURE::DEPTH);
	float* data = (float*) input.data;
	float* ptr_image_num = (float*) ((int8_t*) data + y * input.step);
	float dist = ptr_image_num[x] / 1000.f;
	return dist;
}

