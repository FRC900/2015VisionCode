#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class WriteOnFrame {

	private:
		Mat image;
	public:
		WriteOnFrame(const Mat &setTo);
		void writeTime();
		void writeMatchNumTime(string matchNum, double matchTime);
		void write(VideoWriter &writeTo);
};
