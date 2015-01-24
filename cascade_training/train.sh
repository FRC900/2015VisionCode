#! /bin/bash
/bin/opencv_traincascade -data classifier_bin_5 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages 17 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 7000 -numNeg 5000 -mode ALL -precalcValBufSize 2000 -precalcIdxBufSize 2000 -makWeakCount 500
