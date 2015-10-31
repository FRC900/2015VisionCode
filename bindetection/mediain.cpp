#include "mediain.hpp"
#include <iostream>

using namespace std;

MediaIn::MediaIn()
{
   _frameCounter = 0;
}

int MediaIn::frameCount(void)
{
   return -1;
}

int MediaIn::frameCounter(void)
{
   return _frameCounter;
}

void MediaIn::frameCounter(int frameCount)
{
   _frameCounter = frameCount;
}

double MediaIn::getDepth(int x, int y)
{
   return -1000.;
}
