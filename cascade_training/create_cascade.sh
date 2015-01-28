#! /bin/bash
# $1 = output dir

i=0

while [ -f $1/stage$i.xml ];
do
   i=$((i + 1))
  /bin/opencv_traincascade -data $1 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages $i -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 7000 -numNeg 5000 -mode ALL -precalcValBufSize 2000 -precalcIdxBufSize 2000
  mv $1/cascade.xml $1/cascade_$i.xml
  /bin/opencv_traincascade -data $1 -vec positives.vec -bg negatives.dat -w 20 -h 20 -numStages $i -minHitRate 0.999 -maxFalseAlarmRate 0.5 -numPos 7000 -numNeg 5000 -mode ALL -precalcValBufSize 2000 -precalcIdxBufSize 2000 -baseFormatSave
  cat $1/cascade.xml | sed 's?</cascade>?</output>?' | sed 's?<cascade>?<output type_id=\"opencv-haar-classifier\">?' > $1/cascade_oldformat_$i.xml
done
