#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include "Args.hpp"

using namespace std;

Args::Args(void)
{
	captureAll   = false;
	tracking     = true;
	rects        = true;
	batchMode    = false;
	ds           = false;
	calibrate    = false;
	writeVideo   = false;
	frameStart   = 0.0;
}

bool Args::processArgs(int argc, const char **argv, Args &args)
{
	const string frameOpt      = "--frame=";
	const string captureAllOpt = "--all";
	const string batchModeOpt  = "--batch";
	const string dsOpt         = "--ds";
	const string calibrateOpt  = "--calibrate";
	const string writeVideoOpt = "--capture";
	const string rectsOpt      = "--no-rects";
	const string trackingOpt   = "--no-tracking";
	const string badOpt        = "--";
	// Read through command line args, extract
	// cmd line parameters and input filename
	int fileArgc;
	for (fileArgc = 1; fileArgc < argc; fileArgc++)
	{
		if (frameOpt.compare(0, frameOpt.length(), argv[fileArgc], frameOpt.length()) == 0)
			args.frameStart = atoi(argv[fileArgc] + frameOpt.length());
		else if (captureAllOpt.compare(0, captureAllOpt.length(), argv[fileArgc], captureAllOpt.length()) == 0)
			args.captureAll = true;
		else if (batchModeOpt.compare(0, batchModeOpt.length(), argv[fileArgc], batchModeOpt.length()) == 0)
			args.batchMode = true;
		else if (dsOpt.compare(0, dsOpt.length(), argv[fileArgc], dsOpt.length()) == 0)
			args.ds = true;
		else if (calibrateOpt.compare(0, calibrateOpt.length(), argv[fileArgc], calibrateOpt.length()) == 0)
			args.calibrate = true;
		else if (writeVideoOpt.compare(0, writeVideoOpt.length(), argv[fileArgc], writeVideoOpt.length()) == 0)
			args.writeVideo = true;
		else if (trackingOpt.compare(0, trackingOpt.length(), argv[fileArgc], trackingOpt.length()) == 0)
			args.tracking = false;
		else if (rectsOpt.compare(0, rectsOpt.length(), argv[fileArgc], rectsOpt.length()) == 0)
			args.rects = false;
		else if (badOpt.compare(0, badOpt.length(), argv[fileArgc], badOpt.length()) == 0) // unknown option
		{
			cerr << "Unknown command line option " << argv[fileArgc] << endl;
			return false; 
		}
		else // first non -- arg is filename or camera number
			break;
	}
	if (fileArgc < argc)
		args.inputName = argv[fileArgc];
	return true;
}
