#include <iostream>
#include <sstream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "classifierio.hpp"

using namespace std;

ClassifierIO::ClassifierIO(string baseDir, int dirNum, int stageNum) :
   baseDir_  ( baseDir),
   dirNum_   ( dirNum),
   stageNum_ ( stageNum)
{
}

// given a directory number generate a filename for that dir
// if it exists - if it doesnt, return an empty string
string ClassifierIO::getClassifierDir() const
{
   struct stat fileStat;
   stringstream ss;
   ss << baseDir_ << dirNum_;
   if ((stat(ss.str().c_str(), &fileStat) == 0) && S_ISDIR(fileStat.st_mode))
      return string(ss.str());
   return string();
}

// given a directory number and stage within that directory
// generate a filename to load the cascade from.  Check that
// the file exists - if it doesnt, return an empty string
string ClassifierIO::getClassifierName() const
{
   struct stat fileStat;
   stringstream ss;
   string dirName = getClassifierDir();
   if (!dirName.length())
      return string();

   ss << dirName;
   ss << "/cascade_oldformat_";
   ss << stageNum_;
   ss << ".xml";

   if ((stat(ss.str().c_str(), &fileStat) == 0) && (fileStat.st_size > 5000))
      return string(ss.str());

   // Try the non-oldformat one next
   ss.str(string());
   ss.clear();
   ss << dirName;
   ss << "/cascade_";
   ss << stageNum_;
   ss << ".xml";

   if ((stat(ss.str().c_str(), &fileStat) == 0) && (fileStat.st_size > 5000))
      return string(ss.str());

   // Found neither?  Return an empty string
   return string();
}

// Find the next valid classifier. Since some .xml input
// files crash the GPU we've deleted them. Skip over missing
// files in the sequence
bool ClassifierIO::findNextClassifierStage(bool increment)
{
   int adder = increment ? 1 : -1;
   int num = stageNum_ + adder;
   bool found;

   for (found = false; !found && ((num > 0) && (num < 100)); num += adder)
   {
      ClassifierIO tempClassifier(baseDir_, dirNum_, num);
      if (tempClassifier.getClassifierName().length())
      {
	 *this = tempClassifier; 
	 found = true;
      }
   }
      
   return found;
}

// Find the next valid classifier dir. Start with current stage in that
// directory and work down until a classifier is found
bool ClassifierIO::findNextClassifierDir(bool increment)
{
   int adder = increment ? 1 : -1;
   int dnum = dirNum_ + adder;
   bool found;

   for (found = false; !found && ((dnum > 0) && (dnum < 100)); dnum += adder)
   {
      ClassifierIO tempClassifier(baseDir_, dnum, stageNum_ + 1);
      if (tempClassifier.getClassifierDir().length())
      {
	 // Try to find a valid classifier in this dir, counting
	 // down from the current stage number
	 if (tempClassifier.findNextClassifierStage(false))
	 {
	    *this = tempClassifier;
	    found = true;
	 }
      }
   }
      
   return found;
}

int ClassifierIO::dirNum(void) const
{
   return dirNum_;
}

int ClassifierIO::stageNum(void) const
{
   return stageNum_;
}
