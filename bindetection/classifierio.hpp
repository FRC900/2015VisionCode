#ifndef CLASSIFIERIO_HPP__
#define CLASSIFIERIO_HPP__

#include <string>

class ClassifierIO
{
   public:
		ClassifierIO(std::string baseDir, int dirNum, int stageNum);
		std::string getClassifierDir(void) const;
		std::string getClassifierName(void) const;
		bool findNextClassifierStage(bool increment);
		bool findNextClassifierDir(bool increment);
		int dirNum(void) const;
		int stageNum(void) const;
   private :
		std::string baseDir_;
		int dirNum_;
		int stageNum_;
};

#endif

