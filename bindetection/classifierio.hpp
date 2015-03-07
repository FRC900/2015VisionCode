#ifndef CLASSIFIERIO_HPP__
#define CLASSIFIERIO_HPP__

std::string getClassifierDir(int directory);
std::string getClassifierName(int directory, int stage);
bool findNextClassifierStage(int dirNum, int &stageNum, bool increment);
bool findNextClassifierDir(int &dirNum, int &stageNum, bool increment);



#endif

