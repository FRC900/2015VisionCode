#! /bin/bash
/bin/opencv_traincascade -data classifier_bin_4 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages 10 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 500 -numNeg 1000 -mode ALL -precalcValBufSize 3000 -precalcIdxBufSize 3000

