#ifndef INC_WRITEONFRAME_HPP__
#define INC_WRITEONFRAME_HPP__

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <opencv2/opencv.hpp>

class WriteOnFrame {
	private:
		cv::Mat image;
	public:
		WriteOnFrame(const cv::Mat &setTo);
		void writeTime();
		void writeMatchNumTime(std::string matchNum, double matchTime);
		void write(cv::VideoWriter &writeTo);
};
#endif
