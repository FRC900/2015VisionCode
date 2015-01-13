\cygwin64\bin\opencv_traincascade -data classifier_bin_3 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages 15 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 1200 -numNeg 3100 -mode ALL -precalcValBufSize 3000 -precalcIdxBufSize 3000

