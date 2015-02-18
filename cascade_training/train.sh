#! /bin/bash
/bin/opencv_traincascade -data classifier_bin_7 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages 55 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 9000 -numNeg 10000 -mode ALL -precalcValBufSize 3500 -precalcIdxBufSize 3500 -maxWeakCount 500
