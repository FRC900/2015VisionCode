#include <fstream>
#include <iostream>
#include <ga/GASimpleGA.h>
#include <ga/GA2DBinStrGenome.h>
#include <ga/std_stream.h>
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "imagedetect.hpp"
#include <stdlib.h> 
#include <sys/types.h>
#include <dirent.h>
using namespace std;
using namespace cv;
BaseCascadeDetect *detectCascade;
struct binImage {Mat image; Rect binLoc;};
vector <binImage> allBinImages;
int centerArea = 4000;
int rectArea = 100;
Mat drawCopy;

vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
       if (!item.empty())
        elems.push_back(item);
    }
    return elems;
}

bool rectangleCompare(Rect rect1, Rect rect2) {

Point center1 = Point(rect1.width/2,rect1.height/2);
Point center2 = Point(rect2.width/2,rect2.height/2);
Rect centerRect = Rect(center1,center2);
bool matched = ((abs((rect1.width * rect1.height) - (rect2.width * rect2.height)) < rectArea) && ((centerRect.width * centerRect.height) < centerArea));
if(matched) 
cout << "rectangles matched" << endl;
return matched;
}


vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, elems);
    return elems;
}

// this function returns an int from a binary string
unsigned int getIntFrom2DBinaryString(const GA2DBinaryStringGenome &genome,unsigned int row)
{
     unsigned int intval = 0;
     for(int i=0; i<genome.width(); i++){
        intval <<= 1;
        intval  |= genome.gene(i, row) & 1;
     }
     return intval;
}
//this should return a value depending on how close the GA is to
//the objective


float Objective(GAGenome& g) { 
GA2DBinaryStringGenome & genome = (GA2DBinaryStringGenome &)g;
  unsigned int intval[genome.height()];
  for(int i=0; i<genome.height(); i++){
     intval[i] = getIntFrom2DBinaryString(genome, i);
  }

vector <Rect> binsThreshold;
vector <Rect> binsClassifier;
vector <Rect> filteredBins;
Mat threshHoldImage;
int foundImage = 0;
cout << "trying values: ";
for(int i = 0; i < genome.height(); i++)
cout << intval[i] << ",";
/* scale = intval[0];
neighbors = intval[1]; */
for(int i = 0; i < allBinImages.size(); i++) {

	detectCascade->cascadeDetect(allBinImages[i].image,binsClassifier);
	if (binsClassifier.size() == 1)
	if (rectangleCompare(binsClassifier[0],allBinImages[i].binLoc))
	foundImage++;
	}
drawCopy = allBinImages[allBinImages.size() - 1].image.clone();
rectangle(drawCopy,allBinImages[allBinImages.size() - 1].binLoc,Scalar(0,255,0),4);
rectangle(drawCopy,binsClassifier[allBinImages.size() - 1],Scalar(0,0,255),4);
imshow("Image",drawCopy);
waitKey(5);
float successRate = (float)foundImage / (float)allBinImages.size();
cout << " Successful Images " << foundImage << endl;
return successRate;
}

int main(int argc, char **argv) {

   const char *cascadeName = "../cascade_training/classifier_bin_5/cascade_27.xml";
   if (gpu::getCudaEnabledDeviceCount() > 0)
      detectCascade = new GPU_CascadeDetect(cascadeName);
   else
      detectCascade = new CPU_CascadeDetect(cascadeName);

   //-- 1. Load the cascades
   if( !detectCascade->loaded() )
   {
      cerr << "--(!)Error loading " << cascadeName << endl; 
      return -1; 
   }
for(int ii=1; ii<argc; ii++) {
  if(strcmp(argv[ii++],"seed") == 0) {
    GARandomSeed((unsigned int)atoi(argv[ii]));
  }
}
int imageNum = 0;
int position;
vector<string> image_names;
vector<string> parameters;
DIR *dirp = opendir("./gaSource/");
struct dirent *dp;
if (!dirp)
return -1;
while ((dp = readdir(dirp)) != NULL) {
   if (strstr(dp->d_name, ".png") && !strstr(dp->d_name, "_r.png"))
	 image_names.push_back(dp->d_name);
   }
   closedir(dirp);
   cout << "Read " << image_names.size() << " image names" << endl;
   for (vector<string>::iterator it = image_names.begin(); it != image_names.end(); ++it)
   {
	allBinImages.push_back(binImage());
	string imagePath = "./gaSource/" + *it;
	allBinImages[imageNum].image = imread(imagePath);
	*it = it->substr(0, it->rfind('.'));
	parameters = split(*it,'_');
	if(parameters.size() != 7) {
		cout << "file parameters error" << endl;
		return -1;
		}
	cout << "Image " << it - image_names.begin() << " parameters: ";
	for(int i = 1; i < parameters.size(); i++)
	cout << parameters[i] << " ";
	cout << endl;
	Point rectPoint1 = Point(atoi(parameters[3].c_str()),atoi(parameters[4].c_str())); //top left
	Point rectPoint2 = Point((atoi(parameters[3].c_str()) + atoi(parameters[5].c_str())),(atoi(parameters[4].c_str()) + atoi(parameters[6].c_str())));
	allBinImages[imageNum].binLoc = Rect(rectPoint1,rectPoint2);
	if(allBinImages[imageNum].image.empty()) {
		cout << "image loading error" << endl;
		return -1;
		}
	imageNum++;
    }
int width    = 6; //number of bits per number
int height   = 2; //number of values to change
int popsize  = 30; //how many pairs of numbers it creates per gen
int ngen     = 400; //number of generations to run
float pmut   = 0.001; //chance of mutation
float pcross = 0.9; //chance of crossover


//create the genome
GA2DBinaryStringGenome genome(width, height, Objective);
GASimpleGA ga(genome); //create the algorithm
ga.populationSize(popsize); //set parameters that we defined
ga.nGenerations(ngen);
ga.pMutation(pmut);
ga.pCrossover(pcross);
ga.scoreFilename("iteration_data.dat"); //write stuff to this file
ga.flushFrequency(20); //frequency of writing to data
cout << "created GA, starting now..." << endl;
ga.evolve(); //RUN!
//now we print out the best values that it found
genome = ga.statistics().bestIndividual();
cout << "The GA found:" << endl;
double accum = 0;
  for (int i = 0; i < genome.height(); i++) //for each value
  {
     unsigned int intval = getIntFrom2DBinaryString(genome, i);
     cout << "value " << i << " : " << intval << endl;
  }
}
