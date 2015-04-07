#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include "Args.hpp"

using namespace std;

Args::Args(void)
{
	captureAll         = false;
	tracking           = true;
	rects              = true;
	batchMode          = false;
	ds                 = false;
	calibrate          = false;
	writeVideo         = false;
	classifierDirNum   = 14;
	classifierStageNum = 29;
	frameStart         = 0.0;
}

bool Args::processArgs(int argc, const char **argv)
{
	const string frameOpt           = "--frame=";
	const string captureAllOpt      = "--all";
	const string batchModeOpt       = "--batch";
	const string dsOpt              = "--ds";
	const string calibrateOpt       = "--calibrate";
	const string writeVideoOpt      = "--capture";
	const string rectsOpt           = "--no-rects";
	const string trackingOpt        = "--no-tracking";
	const string classifierDirOpt   = "--classifierDir=";
	const string classifierStageOpt = "--classifierStage=";
	const string badOpt             = "--";
	// Read through command line args, extract
	// cmd line parameters and input filename
	int fileArgc;
	for (fileArgc = 1; fileArgc < argc; fileArgc++)
	{
		if (frameOpt.compare(0, frameOpt.length(), argv[fileArgc], frameOpt.length()) == 0)
			frameStart = atoi(argv[fileArgc] + frameOpt.length());
		else if (captureAllOpt.compare(0, captureAllOpt.length(), argv[fileArgc], captureAllOpt.length()) == 0)
			captureAll = true;
		else if (batchModeOpt.compare(0, batchModeOpt.length(), argv[fileArgc], batchModeOpt.length()) == 0)
			batchMode = true;
		else if (dsOpt.compare(0, dsOpt.length(), argv[fileArgc], dsOpt.length()) == 0)
			ds = true;
		else if (calibrateOpt.compare(0, calibrateOpt.length(), argv[fileArgc], calibrateOpt.length()) == 0)
			calibrate = true;
		else if (writeVideoOpt.compare(0, writeVideoOpt.length(), argv[fileArgc], writeVideoOpt.length()) == 0)
			writeVideo = true;
		else if (trackingOpt.compare(0, trackingOpt.length(), argv[fileArgc], trackingOpt.length()) == 0)
			tracking = false;
		else if (rectsOpt.compare(0, rectsOpt.length(), argv[fileArgc], rectsOpt.length()) == 0)
			rects = false;
		else if (classifierDirOpt.compare(0, classifierDirOpt.length(), argv[fileArgc], classifierDirOpt.length()) == 0)
			classifierDirNum = atoi(argv[fileArgc] + classifierDirOpt.length());
		else if (classifierStageOpt.compare(0, classifierStageOpt.length(), argv[fileArgc], classifierStageOpt.length()) == 0)
			classifierStageNum = atoi(argv[fileArgc] + classifierStageOpt.length());
		else if (badOpt.compare(0, badOpt.length(), argv[fileArgc], badOpt.length()) == 0) // unknown option
		{
			cerr << "Unknown command line option " << argv[fileArgc] << endl;
			return false; 
		}
		else // first non -- arg is filename or camera number
			break;
	}
	if (fileArgc < argc)
		inputName = argv[fileArgc];
	return true;
}
