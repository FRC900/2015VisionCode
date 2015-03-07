#include <iostream>
#include <sstream>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "classifierio.hpp"

using namespace std;

// given a directory number generate a filename for that dir
// if it exists - if it doesnt, return an empty string
string getClassifierDir(int directory)
{
   struct stat fileStat;
   stringstream ss;
   ss << "/home/ubuntu/2015VisionCode/cascade_training/classifier_bin_";
   ss << directory;
   if ((stat(ss.str().c_str(), &fileStat) == 0) && S_ISDIR(fileStat.st_mode))
      return string(ss.str());
   return string();
}

// given a directory number and stage within that directory
// generate a filename to load the cascade from.  Check that
// the file exists - if it doesnt, return an empty string
string getClassifierName(int directory, int stage)
{
   struct stat fileStat;
   stringstream ss;
   string dirName = getClassifierDir(directory);
   if (!dirName.length())
      return string();

   ss << dirName;
   ss << "/cascade_oldformat_";
   ss << stage;
   ss << ".xml";

   if ((stat(ss.str().c_str(), &fileStat) == 0) && (fileStat.st_size > 5000))
      return string(ss.str());

   // Try the non-oldformat one next
   ss.str(string());
   ss.clear();
   ss << dirName;
   ss << "/cascade_";
   ss << stage;
   ss << ".xml";

   if ((stat(ss.str().c_str(), &fileStat) == 0) && (fileStat.st_size > 5000))
      return string(ss.str());

   // Found neither?  Return an empty string
   return string();
}

// Find the next valid classifier. Since some .xml input
// files crash the GPU we've deleted them. Skip over missing
// files in the sequence
bool findNextClassifierStage(int dirNum, int &stageNum, bool increment)
{
   int adder = increment ? 1 : -1;
   int num = stageNum + adder;
   bool found;

   for (found = false; !found && ((num > 0) && (num < 100)); num += adder)
   {
      if (getClassifierName(dirNum, num).length())
      {
	 found = true;
	 stageNum = num;
      }
   }
      
   return found;
}

// Find the next valid classifier dir. Start with current stage in that
// directory and work down until a classifier is found
bool findNextClassifierDir(int &dirNum, int &stageNum, bool increment)
{
   int adder = increment ? 1 : -1;
   int dnum = dirNum + adder;
   bool found;

   for (found = false; !found && ((dnum > 0) && (dnum < 100)); dnum += adder)
   {
      if (getClassifierDir(dnum).length())
      {
	 int snum = stageNum + 1;
	 // Try to find a valid classifier in this dir, counting
	 // down from the current stage number
	 if (findNextClassifierStage(dnum, snum, false))
	 {
	    dirNum   = dnum;
	    stageNum = snum;
	    found    = true;
	 }
      }
   }
      
   return found;
}

