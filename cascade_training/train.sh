#! /bin/bash
/bin/opencv_traincascade -data classifier_bin_6 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages 35 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 7000 -numNeg 6000 -mode ALL -precalcValBufSize 3500 -precalcIdxBufSize 3500 -maxWeakCount 500
