#! /bin/bash
/bin/find positive_images -name \*.png > positives.dat
/bin/find positive_images -name \*.jpg >> positives.dat
/bin/find negative_images -name \*.png > negatives.dat
/bin/find negative_images -name \*.jpg >> negatives.dat
perl createtrainsamples.pl positives.dat negatives.dat . 3800 | tee foo.txt
/bin/find . -name \*.vec > vectors.dat
rm positives.vec
mergevec/src/mergevec.exe vectors.dat ordered_positives.vec
perl randomize_dat.pl ordered_positives.vec positives.vec

