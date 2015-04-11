#ifndef CLASSIFIERIO_HPP__
#define CLASSIFIERIO_HPP__

class ClassifierIO
{
   public:
		ClassifierIO(int dirNum, int stageNum);
		std::string getClassifierDir(void) const;
		std::string getClassifierName(void) const;
		bool findNextClassifierStage(bool increment);
		bool findNextClassifierDir(bool increment);
		int dirNum(void) const;
		int stageNum(void) const;
   private :
		int _dirNum;
		int _stageNum;
};

#endif

