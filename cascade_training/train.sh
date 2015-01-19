#! /bin/bash
/bin/opencv_traincascade -data classifier_bin_5 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages 15 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 7000 -numNeg 5000 -mode ALL -precalcValBufSize 3000 -precalcIdxBufSize 3000

