#ifndef TRACH_HPP_INC__
#define TRACH_HPP_INC__

const size_t TrackedObjectHistoryDistance = 20;
const double HFOV = 70.42;
class TrackedObject
{
   public :
      TrackedObject(int imageWidth, size_t historyLength = TrackedObjectHistoryDistance)
      {
	 _historyLength        = historyLength;
	 _detectHistory      = new bool[historyLength];
	 _distanceHistory        = new double[historyLength];
	 _angleHistory       = new double[historyLength];
	 _historyIndex       = 0;
	 _imageWidth         = imageWidth;
      }

      void adjustAngle(double deltaAngle)
      {
	 for (size_t i = 0; i < _historyLength; i++)
	    _angleHistory[i] += deltaAngle;
	 _position.x += _imageWidth / HFOV;
      }

      void setDistance(double size)
      {
	 _distanceHistory[_historyIndex % _historyLength] = size;
	 setDetected();
      }

      void setDistance(Rect rect)
      {
	 double FOVFrac = (double)rect.width / _imageWidth;
	 double totalFOV = 12.0 / FOVFrac;
	 distanceVal = totalFOV / tan(0.59);
	 setDistance(distanceVal);
      }

      void setAngle(double angle)
      {
	 _angleHistory[_historyIndex % _historyLength] = angle;
	 setDetected();
      }

      void setAngle(Rect rect)
      {
	 double rectCenter = rect.x + rect.cols / 2.0;
	 double deltaX = _imageWidth/2.0 - rectCenter;
	 setAngle ();
      }

      void setDetected(void)
      {
	 _detectHistory[_historyIndex % _historyLength] = true;
      }

      void clearDetected(void)
      {
	 _detectHistory[_historyIndex % _historyLength] = false;
      }

      void nextFrame(void)
      {
	 _historyIndex += 1;
	 clearDetected();
      }

      double distance(Rect point)
      {
	 return (_position.x - point.x) * (_position.x - point.x) +
	        (_position.y - point.y) * (_position.y - point.y);
      }

      double area(void)
      {
	 return _position.width * _position.height;
      }

      // get size, angle average and stddev
      
   private :
      cv::Rect _position;
      int      _imageWidth;
      size_t   _historyLength;
      size_t   _historyIndex;
      bool    *_detectHistory;
      double  *_distanceHistory;
      double  *_angleHistory;
};


// Tracked object array - 
// operator 

#endif

