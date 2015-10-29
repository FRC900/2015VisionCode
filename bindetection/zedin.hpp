//opencv include
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"

//zed include
#include <zed/Mat.hpp>
#include <zed/Camera.hpp>
#include <zed/utils/GlobalDefine.hpp>

//cuda include
#include "cuda.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include "npp.h"
#include "device_functions.h"

#include "videoin_c920.hpp"

class ZedIn: public VideoIn
{
   public:
	ZedIn();
        bool getNextFrame(bool pause, cv::Mat &frame);
	bool getNextFrame(cv::Mat &frame,bool pause,bool left);
	double getDepth(int x, int y);
   private:
	sl::zed::Mat depthMat;
	sl::zed::Camera* zed;
	sl::zed::Mat imageGPU;
};

