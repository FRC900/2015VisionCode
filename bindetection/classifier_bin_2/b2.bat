\cygwin64\bin\opencv_traincascade -data classifier_bin_2 -vec positives.vec -bg negatives.dat -w 30 -h 30 -numStages 20 -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 600 -numNeg 2000 -mode ALL -precalcValBufSize 3000 -precalcIdxBufSize 3000

