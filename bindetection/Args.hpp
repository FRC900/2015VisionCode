#ifndef INC__ARGS_HPP__
#define INC__ARGS_HPP__

#include <string>


class Args //class for processing arguments
{
	public :
		bool captureAll;       // capture all found targets to image files?
		bool tracking;         // display tracking info?
		bool rects;            // display frame by frame hit info
		bool batchMode;        // non-interactive mode - no display, run through
						       // as quickly as possible. Combine with --all?
		bool ds;               // driver-station?
		bool writeVideo;       // write captured video to output
		int  frameStart;       // frame number to start from
		bool calibrate;        // crosshair to calibrate camera
		std::string inputName; // input file name or camera number

		Args(void);
		bool processArgs(int argc, const char **argv, Args &args);
};

#endif
