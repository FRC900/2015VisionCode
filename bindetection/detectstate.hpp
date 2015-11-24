#ifndef DETECT_STATE_HPP__
#define DETECT_STATE_HPP__

#include "classifierio.hpp"
#include "objdetect.hpp"

// A class to manage the currently loaded detector plus the state loaded
// into that detector. Right now it is hardcoded for cascade classifiers
// TODO : figure out how to combine, say, a NN-based classifer as well
class DetectState
{

   public:
      DetectState(const ClassifierIO &classifierIO, bool gpu = false);
      ~DetectState()
      {
	if (detector_)
	   delete detector_;
      }
      bool update(void);
      void toggleGPU(void);
      void changeModel(bool increment);
      void changeSubModel(bool increment);
      std::string print(void) const;
      ObjDetect *detector(void)
      {
	 return detector_;
      }
   private:
      ObjDetect    *detector_;
      ClassifierIO  classifierIO_;
      bool          gpu_;
      bool          reload_;
};

#endif
